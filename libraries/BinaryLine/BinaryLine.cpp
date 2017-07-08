#include <Arduino.h>
#include "BinaryLine.h"
#include "EPD2.h"



BinaryLine::BinaryLine(
	const uint8_t *font,
	const short unsigned int *fontIndex,
        const unsigned char *fontWidth)
{
	this->font = font;
	this->fontIndex = fontIndex;
        this->fontWidth = fontWidth;
}


void BinaryLine::insertMap(unsigned char *mapPoints, int currentHeight, int bitOffset)
{
	for (int i = 0; i < 200; i++) //TODO 200 evtl net hardcoden
	{
		unsigned char y = mapPoints[i * 2 + 1];
		unsigned char x = mapPoints[i * 2];
		if (y >= currentHeight - 1 && y <= currentHeight + 1)
		{
			//Render Points
			for (int i = -1; i <= 1; i++)
			{
				int byteIndex = (x + i + bitOffset) / 8;
				int bitIndex = (x + i + bitOffset) % 8;
				char point = 1 << bitIndex;
				this->line[byteIndex] |= point;
			}
		}
	}
}

void BinaryLine::insertImg(const unsigned char *img, unsigned char currentHeight, const unsigned char width, const unsigned char height, const unsigned char bitOffset, bool flipHorizontal, bool flipVertical)
{
	if (flipHorizontal)
		currentHeight = height - currentHeight - 1;

	unsigned short startBit = width * currentHeight;
	unsigned short currentLineBitIndex = flipVertical ? bitOffset + width : bitOffset;
	unsigned char bitsToRead = width;
	do
	{
		unsigned short startByte = startBit / 8;
		unsigned char readedBits = 8;
		unsigned char character = pgm_read_byte_near(img + startByte);


		//Check if unecceserry ending bits
		if (bitsToRead < 8)
		{
			//Cut the last x bits of since it ist lsb => cut first x bits off
			unsigned char shift = 1 << (8 - bitsToRead);
			character *= shift;
			character /= shift;
			readedBits = bitsToRead;
		}

		//Check if unecceserry starting bits
		if (startBit % 8 != 0)
		{
			unsigned char offset = startBit % 8;
			unsigned short shift = 1 << (offset);
			character /= shift;
			readedBits = 8 - offset;
		}

		if (flipVertical)
			character = reverse(character);
		this->insertByte(character, currentLineBitIndex);

		startBit += readedBits;
		bitsToRead -= readedBits;
		if (flipVertical)
			currentLineBitIndex -= readedBits;
		else
			currentLineBitIndex += readedBits;
	} while (bitsToRead > 0);
}

void BinaryLine::insertText(const char *text, const unsigned char textLength, const unsigned char currentHeight, const unsigned char bitOffset)
{
	unsigned short currentLineBitIndex = bitOffset;
	for (int cIndex = 0; cIndex < textLength; cIndex++)
	{
		//32 is first Character in Font
		unsigned char bitsToRead = pgm_read_byte_near(this->fontWidth + (text[cIndex] - 32));
		unsigned short startByte = this->fontIndex[(text[cIndex] - 32)] + (currentHeight * ceil(bitsToRead / 8.0f));
		do
		{
			unsigned char readedBits = 8;
			unsigned char character = pgm_read_byte_near(this->font + startByte);

			//Check if unecceserry ending bits
			if (bitsToRead < 8)
			{
				//Cut the last x bits of since it ist lsb => cut first x bits off
				unsigned short shift = 1 << (8 - bitsToRead);
				character *= shift;
				character /= shift;
				readedBits = bitsToRead;
			}

			this->insertByte(character, currentLineBitIndex);

			startByte++;
			bitsToRead -= readedBits;
			currentLineBitIndex += readedBits;
		} while (bitsToRead > 0);
		currentLineBitIndex += 2;
	}
}

void BinaryLine::clear()
{
	memset(this->line, 0, 33 * sizeof(unsigned char));
}


void BinaryLine::render(EPD_Class *EPD, unsigned char lineIndex)
{
    EPD->line(lineIndex, this->line, 0x00, false, 2);
}

unsigned char BinaryLine::reverse(unsigned char b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}


void BinaryLine::insertByte(unsigned char character, const unsigned short bitOffset)
{
	unsigned char lineIndex = bitOffset / 8;
	if (bitOffset % 8 == 0)
	{
		if (lineIndex < 33)
			this->line[lineIndex] = character;
	}
	else
	{
		int offset = bitOffset % 8;
		unsigned char copy = character;
		character = character << offset;
		copy = copy >> (8 - offset);

		if (lineIndex < 32)
		{
			this->line[lineIndex] |= character;
			this->line[lineIndex + 1] |= copy;
		}
	}
}
