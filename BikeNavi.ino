#include <inttypes.h>
#include <ctype.h>

/**************
      Libs
 *************/
#include <SPI.h>
#include <FLASH.h>
#include <EPD2.h>
#include <BinaryLine.h>
#include <Wire.h>
#include <LM75.h>


/**************
      FONT
 *************/
#include <Fonts.h>  // this is just an empty file
#define MY_FONT calibri_14pt_bitmap
#define MY_FONT_WIDTH calibri_14pt_width
#define MY_FONT_INDEX calibri_14pt_index
#define FONT_CHARACTER_HEIGHT 17

PROGMEM
#define unsigned
#define char uint8_t
#include "calibri.c"
#undef char
#undef unsigned

PROGMEM
#include "calibri_width.c"

#include "calibri_index.c"



/****************
      RENDERER
 ***************/
#define LINE_WIDTH  264
#define TOTAL_LINES 176
#define REPEAT 3
#define STEP 2
#define BLOCK 16

/****************
      IMAGES
 ***************/
#include <Images.h>  // this is just an empty file
#define IMG_ARROW_WIDTH 32
#define IMG_ARROW_HEIGHT 40
#define IMG_NORTH_WIDTH 16
#define IMG_NORTH_HEIGHT 16
#define LEFT_IMG arrow_left_bits
#define RIGHT_IMG arrow_right_bits
#define STRAIGHT_IMG arrow_straight_bits
#define NORTH_IMG_0 n_0_bits
#define NORTH_IMG_45 n_45_bits
#define NORTH_IMG_90 n_90_bits

PROGMEM const
#define unsigned
#define char uint8_t
#include "left.xbm"
#undef char
#undef unsigned

PROGMEM const
#define unsigned
#define char uint8_t
#include "right.xbm"
#undef char
#undef unsigned

PROGMEM const
#define unsigned
#define char uint8_t
#include "straight.xbm"
#undef char
#undef unsigned

PROGMEM const
#define unsigned
#define char uint8_t
#include "n_0.xbm"
#undef char
#undef unsigned

PROGMEM const
#define unsigned
#define char uint8_t
#include "n_45.xbm"
#undef char
#undef unsigned

PROGMEM const
#define unsigned
#define char uint8_t
#include "n_90.xbm"
#undef char
#undef unsigned


/************************
      RENDER-UTILITY
 ************************/
unsigned char mapPoints[400] = {
  184, 94,
  180, 94,
  179, 93,
  178, 93,
  176, 92,
  174, 91,
  170, 89,
  166, 87,
  161, 85,
  157, 83,
  154, 80,
  153, 80,
  151, 79,
  150, 78,
  147, 74,
  143, 70,
  139, 67,
  136, 65,
  133, 63,
  130, 61,
  127, 59,
  124, 57,
  121, 55,
  119, 53,
  116, 51,
  113, 49,
  111, 48,
  108, 48,
  106, 47,
  103, 47,
  101, 47,
  99, 47,
  94, 48,
  89, 49,
  88, 49,
  81, 50,
  74, 52,
  70, 53,
  68, 53,
  63, 54,
  59, 56,
  53, 58,
  49, 59,
  43, 61,
  37, 64,
  36, 65,
  31, 67,
  27, 70,
  24, 72,
  21, 74,
  19, 76,
  18, 77,
  16, 78,
  16, 78,
  15, 78,
  15, 78,
  15, 77,
  15, 77,
  16, 75,
  16, 74,
  17, 73,
  18, 72,
  19, 70,
  20, 68,
  21, 66,
  22, 64,
  24, 61,
  25, 58,
  27, 56,
  29, 53,
  30, 52,
  32, 49,
  34, 43,
  35, 42,
  35, 41,
  36, 40,
  38, 39,
  40, 38,
  42, 36,
  46, 33,
  53, 28,
  60, 22,
  65, 18,
  70, 13,
  74, 10,
  77, 7,
  79, 6,
  80, 4,
  81, 3,
  81, 2,
  82, 1,
  0x00
}; //200 possilbe points
/***************************
      Arduino IO layout
***************************/
//#define Pin_TEMPERATURE = A0; // Temperature is handled by LM75 over I2C and not an analog pin
#define Pin_MICROSD_CS 3
#define Pin_PANEL_ON 5
#define Pin_BORDER 10
#define Pin_DISCHARGE 4
//#define Pin_PWM 5  // Not used by COG v2
#define Pin_RESET 6
#define Pin_BUSY 7
#define Pin_EPD_CS 8
#define Pin_FLASH_CS 9
#define Pin_SW2 12
#define Pin_RED_LED 13

// LED anode through resistor to I/O pin
// LED cathode to Ground
#define LED_ON  HIGH
#define LED_OFF LOW

// I/O setup
void setup() {
  Serial.begin(9600);
  Serial.println("Ia");
  pinMode(Pin_RED_LED, OUTPUT);
  //pinMode(Pin_SW2, INPUT);
  //pinMode(Pin_TEMPERATURE, INPUT);
  //pinMode(Pin_PWM, OUTPUT);
  pinMode(Pin_BUSY, INPUT);
  pinMode(Pin_RESET, OUTPUT);
  pinMode(Pin_PANEL_ON, OUTPUT);
  pinMode(Pin_DISCHARGE, OUTPUT);
  pinMode(Pin_BORDER, OUTPUT);
  pinMode(Pin_EPD_CS, OUTPUT);
  pinMode(Pin_FLASH_CS, OUTPUT);
  pinMode(Pin_MICROSD_CS, OUTPUT);
  Serial.println("Pin Modes defined");

  digitalWrite(Pin_RED_LED, LOW);
  //digitalWrite(Pin_PWM, LOW);  // not actually used - set low so can use current eval board unmodified
  digitalWrite(Pin_RESET, LOW);
  digitalWrite(Pin_PANEL_ON, LOW);
  digitalWrite(Pin_DISCHARGE, LOW);
  digitalWrite(Pin_BORDER, LOW);
  digitalWrite(Pin_EPD_CS, LOW);
  digitalWrite(Pin_FLASH_CS, HIGH);
  digitalWrite(Pin_MICROSD_CS, HIGH);
  Serial.println("Initial Pin values Set");
  
  FLASH.begin(Pin_FLASH_CS);
  if (FLASH.available()) {
    Serial.println("FLASH chip detected OK");
  } else {
    uint8_t maufacturer;
    uint16_t device;
    FLASH.info(&maufacturer, &device);
    Serial.print("unsupported FLASH chip: MFG: 0x");
    Serial.print(maufacturer, HEX);
    Serial.print("  device: 0x");
    Serial.print(device, HEX);
    Serial.println();
  }

  // configure temperature sensor
  Wire.begin();
  LM75.begin();
}

// main loop
void loop() {

  char description[] = "Leicht links abbiegen auf Berndshöfer";
  char distance_1[] = "2.3km";
  char distance_2[] = "975.5m";
  unsigned char descriptionLength = sizeof(description) / sizeof(unsigned char) - 1;
  unsigned char distance_1_length = sizeof(distance_1) / sizeof(unsigned char) - 1;
  unsigned char distance_2_length = sizeof(distance_2) / sizeof(unsigned char) - 1;
  unsigned short angle = 315;

  render(description, descriptionLength, distance_1, distance_1_length, distance_2, distance_2_length, angle);

  // flash LED for 5 seconds
  for (unsigned char x = 0; x < 50; ++x) {
    digitalWrite(Pin_RED_LED, LED_ON);
    delay(50);
    digitalWrite(Pin_RED_LED, LED_OFF);
    delay(50);
  }
}



void render(const char *description, unsigned char descriptionLength, const char *distance_1, unsigned char distance_1_length, const char *distance_2, unsigned char distance_2_length, unsigned short angle)
{
  // define the E-Ink display
  EPD_Class EPD(EPD_2_7, Pin_PANEL_ON, Pin_BORDER, Pin_DISCHARGE, Pin_RESET, Pin_BUSY, Pin_EPD_CS);
  BinaryLine line(MY_FONT, MY_FONT_INDEX, MY_FONT_WIDTH);
  short temperature = LM75.read();

  digitalWrite(Pin_MICROSD_CS, HIGH);
  EPD.begin(); // power up the EPD panel

  if (!EPD) {
    Serial.print("EPD error = ");
    Serial.print(EPD.error());
    Serial.println("");
    return;
  }

  EPD.setFactor(temperature); // adjust for current temperature
  EPD.clear();

  for (short n = 0; n < REPEAT; ++n) {

    short BLOCK_begin = 0;
    unsigned char  BLOCK_end = 0;

    while (BLOCK_begin < TOTAL_LINES) {
      BLOCK_end += STEP;
      BLOCK_begin = BLOCK_end - BLOCK;
      
      if (BLOCK_begin < 0) {
        BLOCK_begin = 0;
      }
      else if (BLOCK_begin >= TOTAL_LINES) {
        break;
      }

      bool full_BLOCK = (BLOCK_end - BLOCK_begin == BLOCK);

      for (unsigned char lineIndex = BLOCK_begin; lineIndex < BLOCK_end; ++lineIndex) {
        if (lineIndex >= TOTAL_LINES) {
          break;
        }
        
        if (full_BLOCK && (lineIndex < (BLOCK_begin + STEP))) {
          EPD.line(lineIndex, 0, 0x00, false, EPD_normal);
        } 
        else {
          //Clear Line
          line.clear();

          //Description
          renderText(description, descriptionLength, 0, 0, &line, lineIndex);
          renderHorizontallLine(1 + FONT_CHARACTER_HEIGHT, 88, LINE_WIDTH, &line, lineIndex);
          
          //Direction 1
          renderImage(LEFT_IMG, IMG_ARROW_WIDTH, IMG_ARROW_HEIGHT, 25, 25,  &line, lineIndex);  
          renderText(distance_1, distance_1_length, 0, 80, &line, lineIndex); 
          renderHorizontallLine(97, 0, 88, &line, lineIndex); //Direction Seperator
          
          //Direction 2
          renderImage(RIGHT_IMG, IMG_ARROW_WIDTH, IMG_ARROW_HEIGHT, 25, 105,  &line, lineIndex);  
          renderText(distance_2, distance_2_length, 0, 160, &line, lineIndex);      
          
          //Map
          renderVerticalLine(80, 1 + FONT_CHARACTER_HEIGHT, TOTAL_LINES, &line, lineIndex);   
          renderCompas(100, 25, angle, &line, lineIndex);   
          if (lineIndex > 19)
            line.insertMap(mapPoints, lineIndex - 19, 81);
            
          //Map Center            
          if (lineIndex >= 95 && lineIndex <= 99)
          {
              line.insertByte(0xF0, 168);
              line.insertByte(0x01, 176);
          }

          line.render(&EPD, lineIndex);
        }
      }
    }
  }
  EPD.end();   // power down the EPD panel
}

void renderHorizontallLine(unsigned short y, BinaryLine *line, unsigned char currentLineIndex)
{
  renderHorizontallLine(y, 0, LINE_WIDTH, line, currentLineIndex);
}

void renderHorizontallLine(unsigned short y, unsigned short startX, unsigned short endX, BinaryLine *line, unsigned char currentLineIndex)
{
  //TODO auch innerhalb eines bytes anfangen
  if (currentLineIndex == y) {
    unsigned short startByte = startX / 8;
    unsigned short endByte = endX / 8;
    for (unsigned short index = startByte; index < endByte; index++) {
      line->insertByte(0xFF, index*8); 
    }
  }
}
void renderCompas(unsigned short x, unsigned short y, unsigned short angle, BinaryLine *line, unsigned char currentLineIndex)
{
  if (currentLineIndex > y && currentLineIndex < y + IMG_NORTH_HEIGHT)
  {
    unsigned char *img = NORTH_IMG_0;
    bool flipVertical = false;
    bool flipHorizontal = false;
    
    if((angle > 22 && angle <=67) || 
       (angle > 112 && angle <= 157) || 
       (angle > 202 && angle <= 247) || 
       (angle > 292 && angle <= 337))
    {
      img = NORTH_IMG_45;
    }

    if((angle > 67 && angle <=112) || 
       (angle > 247 && angle <= 292))
    {
      img = NORTH_IMG_90;
    }
    
    if(angle > 112 && angle <=247)
      flipHorizontal = true;
    
    if(angle > 202 && angle <=337)
      flipVertical = true;
 
    line->insertImg(img, currentLineIndex - y, IMG_NORTH_WIDTH, IMG_NORTH_HEIGHT, x, flipHorizontal, flipVertical);
  }

}
void renderVerticalLine(unsigned short x, BinaryLine *line, unsigned char currentLineIndex)
{
  renderVerticalLine(x, 0, TOTAL_LINES, line, currentLineIndex);
}

void renderVerticalLine(unsigned short x, unsigned short startY, unsigned short endY, BinaryLine *line, unsigned char currentLineIndex)
{
    if (currentLineIndex >= startY && currentLineIndex <= endY)
      line->insertByte(0x80, x);
}

void renderImage(unsigned char * img, unsigned int imgWidth, unsigned int imgHeight, unsigned short x, unsigned short y, BinaryLine *line, unsigned char currentLineIndex)
{
  if (currentLineIndex >= y && currentLineIndex < y + imgHeight)
    line->insertImg(img, currentLineIndex - y, imgWidth, imgHeight, x ,false, false);
}


void renderText(unsigned char* text, unsigned char textLength, unsigned short x, unsigned short y, BinaryLine *line, unsigned char currentLineIndex)
{
  if (currentLineIndex >= y && currentLineIndex < y + FONT_CHARACTER_HEIGHT)
    line->insertText(text, textLength, currentLineIndex - y, 0);
}


