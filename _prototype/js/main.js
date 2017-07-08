/**
 * Created by Marian on 13.10.16.
 */

var map;
var pathPoints = [];
var turnPoints = [];

var leftMarkerImage;
var rightMarkerImage;
var straightMarkerImage;
var helperLine;
var helperLineMiniMap;
var helperLineMinimapTop;

function initMap() {
    map = new google.maps.Map(document.getElementById('map'), {
        center: {lat: -34.397, lng: 150.644},
        zoom: 8
    });

    leftMarkerImage = {
        url: 'assets/arrows/left.png',
        size: new google.maps.Size(30, 40),
        origin: new google.maps.Point(0, 0),
        anchor: new google.maps.Point(15, 40)
    };

    rightMarkerImage = {
        url: 'assets/arrows/right.png',
        size: new google.maps.Size(30, 40),
        origin: new google.maps.Point(0, 0),
        anchor: new google.maps.Point(15, 40)
    };

    straightMarkerImage = {
        url: 'assets/arrows/straight.png',
        size: new google.maps.Size(30, 40),
        origin: new google.maps.Point(0, 0),
        anchor: new google.maps.Point(15, 40)
    };

    helperLine = new google.maps.Polyline({
        strokeColor: '#000000',
        geodesic: true,
        strokeOpacity: 1.0,
        strokeWeight: 3
    });
    helperLine.setMap(map);

    helperLineMiniMap = new google.maps.Polyline({
        strokeColor: '#808080',
        geodesic: true,
        strokeOpacity: 1.0,
        strokeWeight: 3
    });
    helperLineMiniMap.setMap(map);

    helperLineMinimapTop = new google.maps.Polyline({
        strokeColor: '#0000ff',
        geodesic: true,
        strokeOpacity: 1.0,
        strokeWeight: 3
    });
    helperLineMinimapTop.setMap(map);


    google.maps.event.addListenerOnce(map,"projection_changed", function() {
        loadData();
    });
}


function convertData()
{
	var linebreak = "\r\n";
	var data = pathPoints.length + linebreak;
	var data += turnPoints.length + linebreak;
	for(var i = 0; i < pathPoints.length; i++)
	{
		data += pathPoints[i].lat + linebreak;
		data += pathPoints[i].lng + linebreak;
		data += pathPoints[i].distance + linebreak;
	}
	for(var i = 0; i < turnPoints.length; i++)
	{
		data += turnPoints[i].type.replace(/[Ll]inks/g, "L").replace(/[Rr]echts/g, "R").replace(/[Ss]traight/g, "S") + linebreak;
		data += getPathPointIndexByTurnPoint(turnPoints[i]) + linebreak;
		data += turnPoints[i].description.replace(/[Ll]inks/g, "L").replace(/[Rr]echts/g, "R") + linebreak;
	}
	return data;
}

function downloadData(link)
{
	console.log("test");
	var data = convertData();
    link.href = 'data:text/plain;charset=utf-8,' + encodeURIComponent(data);
}


function loadData() {
    $.ajax({
        type: "GET" ,
        url: "assets/Auf_ihr_Buben_in_die_Pfalz.tcx" ,
        dataType: "xml" ,
        success: function(xml) {

            var halfwayPoint = Math.floor($(xml).find("Trackpoint").length / 2);

            //Path
            $(xml).find("Trackpoint").each(function (index) {
                var lat = parseFloat($(this).find("Position").find("LatitudeDegrees").text());
                var long = parseFloat($(this).find("Position").find("LongitudeDegrees").text());
                var dist = parseFloat($(this).find("DistanceMeters").text());
                pathPoints.push({lat: lat, lng: long, distance: dist});

                if(index == halfwayPoint)
                {
                    map.setCenter({lat: lat, lng: long});
                }

            });

            var path = new google.maps.Polyline({
                path: pathPoints,
                geodesic: true,
                strokeColor: '#FF0000',
                strokeOpacity: 1.0,
                strokeWeight: 2
            });
            path.setMap(map);


            //Turn Information
            $(xml).find("CoursePoint").each(function () {
                var type = $(this).find("PointType").text();
                var description = $(this).find("Notes").text();
                var lat = parseFloat($(this).find("Position").find("LatitudeDegrees").text());
                var long = parseFloat($(this).find("Position").find("LongitudeDegrees").text());

                var turnInformation = {
                    type: type,
                    description: description,
                    position: {lat: lat, lng: long}
                };
                turnPoints.push(turnInformation);
				
                var infowindow = new google.maps.InfoWindow({
                    content: description
                });

                var icon = straightMarkerImage;
                if(type == "Left")
                    icon = leftMarkerImage;
                else if(type == "Right")
                    icon = rightMarkerImage;

                var marker = new google.maps.Marker({
                    position: {lat: lat, lng: long},
                    map: map,
                    icon: icon,
                    title: type
                });
                marker.addListener('click', function() {
                    infowindow.open(map, marker);
                });

            });

            map.addListener('click', setUsersGPSCoord);
        }
    });
}

function getPathPointIndexByTurnPoint(turnPoint)
{
	for(var i = 0; i < pathPoints.length; i++)
	{
		if(pathPoints[i].lat == turnPoint.position.lat && pathPoints[i].lng == turnPoint.position.lng)
			return i;
	}
	return null;
}

function getNextTurnPointWithDistanceInformationByPathPoint(pathPoint, offset, skip) {
    var startIndex = pathPoints.indexOf(pathPoint);
    var skiped = 0;
    for(var i = startIndex + offset; i < pathPoints.length; i++)
    {
        for(var k = 0; k < turnPoints.length; k++)
        {
            if(pathPoints[i].lat == turnPoints[k].position.lat && pathPoints[i].lng == turnPoints[k].position.lng)
            {
                if(skiped ==  skip)
                {
                    var result = turnPoints[k];
                    result.distance = pathPoints[i].distance;
                    return result;
                }
                else
                {
                    skiped++;
                }
            }
        }
    }
    return null;
}

function setUsersGPSCoord(event) {
    var path = [];
    var clickedPoint = {lat: event.latLng.lat(), lng: event.latLng.lng()};
    path.push(clickedPoint);
    helperLine.setPath(path)


    var minDistance = -1;
    var nearestPointIndex = 0;

    for(var i =  0; i < pathPoints.length; i++)
    {

        var distance = getDistance(clickedPoint, pathPoints[i]);
        if(distance < minDistance || minDistance == -1)
        {
            minDistance = distance;
            nearestPointIndex = i;
        }
    }
    var nearestPoint = pathPoints[nearestPointIndex];



    //Since the path is only marked with rough points we must interpolate the real nearest point!
    //Greedy approach. Just divide the segment before and after in x steps and check again...

    //Todo check if nearestPointIndex > 0 and nearestPointIndex < pathPoints.length
    var steps = 100;
    var offset = 1;
    var pointBefore = pathPoints[nearestPointIndex-1];
    var centerPoint = pathPoints[nearestPointIndex];
    var pointAfter = pathPoints [nearestPointIndex+1];
    var v1 = {lat: (centerPoint.lat - pointBefore.lat)/steps, lng: (centerPoint.lng -pointBefore.lng)/steps, distance: (centerPoint.distance - pointBefore.distance)/steps};
    var v2 = {lat: (pointAfter.lat - centerPoint.lat)/steps, lng: (pointAfter.lng -centerPoint.lng)/steps, distance: (pointAfter.distance - centerPoint.distance)/steps};

    for(var i = 0; i <= steps; i++)
    {
        var interpoltedPointBefor = {lat: pointBefore.lat + (i*v1.lat),lng: pointBefore.lng + (i*v1.lng),distance: pointBefore.distance + (i*v1.distance)};
        var distance = getDistance(clickedPoint, interpoltedPointBefor);
        if(distance < minDistance)
        {
            minDistance = distance;
            nearestPoint = interpoltedPointBefor;
            offset = 0;
        }
    }
    for(var i = 0; i <= steps; i++)
    {
        var interpoltedPointAfter = {lat: centerPoint.lat + (i*v2.lat),lng: centerPoint.lng + (i*v2.lng),distance: centerPoint.distance + (i*v2.distance)};
        var distance = getDistance(clickedPoint, interpoltedPointAfter);
        if(distance < minDistance)
        {
            offset = 1;  //when searching the next turn point dont start with the current point (might be the last turn point)
            minDistance = distance;
            nearestPoint = interpoltedPointAfter;
        }
    }
    var nextTurnPoint = getNextTurnPointWithDistanceInformationByPathPoint(pathPoints[nearestPointIndex],offset, 0);
    var turnPointAfterNext = getNextTurnPointWithDistanceInformationByPathPoint(pathPoints[nearestPointIndex],offset, 1);

    //Draw helper line
    path.push(nearestPoint);
    path.push(nextTurnPoint.position);
    helperLine.setPath(path);

    //Get next Instruction
    var nextTurnImgUrl = "assets/arrows/"+nextTurnPoint.type.toLowerCase()+".png";
    var ridingDistance =  getDistanceToString(nextTurnPoint.distance - nearestPoint.distance);
    var afterNextTurnImgUrl = "assets/arrows/"+turnPointAfterNext.type.toLowerCase()+".png";
    var afterNextridingDistance =  getDistanceToString(turnPointAfterNext.distance - nextTurnPoint.distance);

    //get the minimap
    var minimapSize = 0.5; //in km

    //check if the driver is lost
    var lost = getDistance(clickedPoint, nearestPoint) > minimapSize * 1000;
    if(lost)
    {
        minimapSize = getDistance(clickedPoint, nearestPoint) / 1000 + 0.5;
    }


    var longitudeSpacer = Math.cos(rad(clickedPoint.lat))*40075;
    var rectangleLatitudeDifrence = minimapSize * (360/40075);
    var rectangleLongitudeDifrence = minimapSize  * (360/longitudeSpacer);

    //cos(53.38292839)*40075 = [approx] 23903.297 km long. So 1 km is 1 * (360/23903.297) = 0.015060 degrees.


    var topLeftCorner = {lat: clickedPoint.lat + rectangleLatitudeDifrence, lng: clickedPoint.lng - rectangleLongitudeDifrence};
    var topRightCorner = {lat: clickedPoint.lat + rectangleLatitudeDifrence, lng: clickedPoint.lng + rectangleLongitudeDifrence};
    var buttomLeftCorner = {lat: clickedPoint.lat - rectangleLatitudeDifrence, lng: clickedPoint.lng - rectangleLongitudeDifrence};
    var buttomRightCorner = {lat: clickedPoint.lat - rectangleLatitudeDifrence, lng: clickedPoint.lng + rectangleLongitudeDifrence};

    var dy = nextTurnPoint.position.lat - nearestPoint.lat;
    var dx = Math.cos(Math.PI/180*nearestPoint.lat)*(nextTurnPoint.position.lng - nearestPoint.lng);
    var angle = Math.atan2(dy, dx) * 180 / Math.PI - 90;

    topLeftCorner = rotatePoint(topLeftCorner,clickedPoint,angle);
    topRightCorner = rotatePoint(topRightCorner,clickedPoint,angle);
    buttomLeftCorner = rotatePoint(buttomLeftCorner,clickedPoint,angle);
    buttomRightCorner = rotatePoint(buttomRightCorner,clickedPoint,angle);

    var TopLeftToTopRight = {lat: topRightCorner.lat - topLeftCorner.lat, lng: topRightCorner.lng - topLeftCorner.lng};
    var TopLeftToBottomLeft  = {lat: buttomLeftCorner.lat - topLeftCorner.lat, lng: buttomLeftCorner.lng - topLeftCorner.lng};

    var canvas = $("#minimap");
    var canvasHeight = canvas.height();
    var canvasWidth = canvas.width();
    var ctx = canvas[0].getContext("2d");
    ctx.canvas.width = canvasWidth;
    ctx.canvas.height = canvasHeight;
    ctx.scale(1,1);
    ctx.clearRect(0,0,canvasWidth,canvasHeight);
	
	var renderedPoints = [];
    for(var i = 1; i < pathPoints.length-1; i++)
    {

        var p = pathPoints[i];
        if(insidePolygon(p, [topLeftCorner, topRightCorner, buttomRightCorner, buttomLeftCorner]))
        {
			
            var projection = projectPoint(p, topLeftCorner, TopLeftToTopRight, TopLeftToBottomLeft, canvasWidth, canvasHeight);
            ctx.fillRect(projection.x-1,projection.y-1,3,3);
			
			var inArray = false;
			for(var z = 1; z < renderedPoints.length-1; z++)
			{
				if(renderedPoints[z].x == projection.x && renderedPoints[z].y == projection.y)
				{
					inArray = true;
				}
			}
			if(!inArray)
				renderedPoints.push(projection);
			
					
            /*//interpolate more points after
            var v = {lat: (pathPoints[i+1].lat - p.lat) / 10, lng: (pathPoints[i+1].lng - p.lng) / 10};
            for(var k = 0; k < 10; k++)
            {
                var _p = {lat: p.lat + (k*v.lat),lng: p.lng + (k*v.lng)};
                var projection = projectPoint(_p, topLeftCorner, TopLeftToTopRight, TopLeftToBottomLeft, canvasWidth, canvasHeight);
                ctx.fillRect(projection.x,projection.y,1,1);
            }

            //interpolate more points before
            var v = {lat: (pathPoints[i-1].lat - p.lat) / 10, lng: (pathPoints[i-1].lng - p.lng) / 10};
            for(var k = 0; k < 10; k++)
            {
                var _p = {lat: p.lat + (k*v.lat),lng: p.lng + (k*v.lng)};
                var projection = projectPoint(_p, topLeftCorner, TopLeftToTopRight, TopLeftToBottomLeft, canvasWidth, canvasHeight);
                ctx.fillRect(projection.x,projection.y,1,1);
            }*/
		}
    }
	
    var projectedCenter = {x: Math.floor(canvasWidth / 2), y: Math.floor(canvasHeight / 2)};
    ctx.fillRect(projectedCenter.x-4,projectedCenter.y-4,8,8);

    var img = new Image;
    img.onload = function(){
        ctx.translate(10,10);
        ctx.translate(5,9);
        ctx.rotate(rad(angle));
        ctx.drawImage(img,-5,-9); // Or at whatever offset you like
    };
    img.src = "assets/north.png";



    //warning
    if(nextTurnPoint.distance - nearestPoint.distance < 200 && !lost)
    {
        console.log("test");
        $("#screen").addClass("blinking");
    }
    else
    {
        $("#screen").removeClass("blinking");
    }

    //var draw minimap helperline
    var topPath = [];
    var rectanglePath = [];
    topPath.push(topLeftCorner);
    topPath.push(topRightCorner);
    rectanglePath.push(topRightCorner);
    rectanglePath.push(buttomRightCorner);
    rectanglePath.push(buttomLeftCorner);
    rectanglePath.push(topLeftCorner);
    helperLineMiniMap.setPath(rectanglePath);
    helperLineMinimapTop.setPath(topPath);


    if(lost)
    {
        $("#description").html("Lost Track.. (Distance "+getDistanceToString(getDistance(clickedPoint, nearestPoint))+")");
        $("#currentDirection").html("");
        $("#nextDirection").html("");
    }
    else
    {
        //set display things
        $("#description").html(nextTurnPoint.description);
        $("#currentDirection").html("<img src='"+nextTurnImgUrl+"'><br><div>"+ridingDistance+"</div>");
        $("#nextDirection").html("<img src='"+afterNextTurnImgUrl+"'><br><div>"+afterNextridingDistance+"</div>");
    }

}

var projectPoint = function(point, origin, TopLeftToTopRight, TopLeftToBottomLeft, pixelWidht, pixelHeight)
{
    point = {lat: point.lat - origin.lat, lng: point.lng - origin.lng};

    var l = (point.lat - (point.lng*TopLeftToTopRight.lat/TopLeftToTopRight.lng)) / (-(TopLeftToBottomLeft.lng * TopLeftToTopRight.lat / TopLeftToTopRight.lng) + TopLeftToBottomLeft.lat);
    var k = (point.lng - (l * TopLeftToBottomLeft.lng)) / TopLeftToTopRight.lng;

    var x = Math.floor(k * pixelWidht);
    var y = Math.floor(l * pixelHeight);

    return {x: x, y:y};
};

var rotatePoint = function(point, origion, degree)
{
    var x =  origion.lng   + (Math.cos(rad(degree)) * (point.lng - origion.lng) - Math.sin(rad(degree))  * (point.lat - origion.lat) / Math.abs(Math.cos(rad(origion.lat))));
    var y = origion.lat + (Math.sin(rad(degree)) * (point.lng - origion.lng) * Math.abs(Math.cos(rad(origion.lat))) + Math.cos(rad(degree))   * (point.lat - origion.lat));
    return {lat: y, lng: x};
};

var getDistanceToString = function(distance)
{
    if(distance < 1000)
    {
        return (Math.round( distance * 10) / 10) + " m";
    }
    return (Math.round( distance / 1000 * 10) / 10) + " km";
};

var rad = function(x){
    return x * Math.PI / 180;
};

var insidePolygon = function(location,polygonPoints)
{
    var lastPoint = polygonPoints[polygonPoints.length - 1];
    var isInside = false;
    var x = location.lng;
    polygonPoints.forEach(function (point) 
    {
        var x1 = lastPoint.lng;
        var x2 = point.lng;
        var dx = x2 - x1;

        if (Math.abs(dx) > 180.0)
        {
            // we have, most likely, just jumped the dateline (could do further validation to this effect if needed).  normalise the numbers.
            if (x > 0)
            {
                while (x1 < 0)
                    x1 += 360;
                while (x2 < 0)
                    x2 += 360;
            }
            else
            {
                while (x1 > 0)
                    x1 -= 360;
                while (x2 > 0)
                    x2 -= 360;
            }
            dx = x2 - x1;
        }

        if ((x1 <= x && x2 > x) || (x1 >= x && x2 < x))
        {
            var grad = (point.lat - lastPoint.lat) / dx;
            var intersectAtLat = lastPoint.lat + ((x - x1) * grad);

            if (intersectAtLat > location.lat)
                isInside = !isInside;
        }
        lastPoint = point;
    });

    return isInside;
};

var getDistance = function(p1, p2) {
    var R = 6378137; // Earth’s mean radius in meter
    var dLat = rad(p2.lat - p1.lat);
    var dLong = rad(p2.lng - p1.lng);

    var a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
        Math.cos(rad(p1.lat)) * Math.cos(rad(p2.lat)) *
        Math.sin(dLong / 2) * Math.sin(dLong / 2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    var d = R * c;
    return d; // returns the distance in meter
};