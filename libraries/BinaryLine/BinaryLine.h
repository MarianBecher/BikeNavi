#if !defined(EPD2E_H)
#define EPD2E_H 1

#include <Arduino.h>
#include "../EPD2/EPD2.h"

class BinaryLine {
private:
	const uint8_t *font;
	const short unsigned int *fontIndex;
        const unsigned char *fontWidth;
        unsigned char line[33] = { 0x00 };
	unsigned char reverse(unsigned char b);
public:
        BinaryLine(
		const uint8_t *font,
		const short unsigned int *fontIndex,
                const unsigned char *fontWidth);

        void insertByte(unsigned char character, const unsigned short bitOffset);
	void insertMap(unsigned char *mapPoints, int currentHeight, int bitOffset);
	void insertText(const char *text, const unsigned char textLength, const unsigned char currentHeight, const unsigned char bitOffset);
	void insertImg(const unsigned char *img, unsigned char currentHeight, const unsigned char width, const unsigned char height, const unsigned char bitOffset, bool flipHorizontal, bool flipVertical);
	void clear();
        void render(EPD_Class *EDP, unsigned char lineIndex);
};
#endif
