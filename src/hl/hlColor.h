#include"hlColorSpace.h"

#ifndef __HL_COLOR_H__
#define __HL_COLOR_H__

typedef struct hl_color {
	hlCS cs;
	uint8_t color[HL_MAX_BYTEPERPIXEL];
} hlColor;

hlColor hlNewColor(hlCS cs,
		float a, 
		float b, 
		float c, 
		float d, 
		float alpha);
void  hlColorSetChan(hlColor* color, int channel, float value);
float hlColorGetChan(const hlColor* color, int channel);
hlCS  hlColorGetCS(const hlColor* color);
uint8_t *hlColorGetData(hlColor *color);
void  hlColorCopy(hlColor* dst, const hlColor *src);
void  hlRgbToHsl(float *rgb, float *hsl);
void  hlHslToRgb(float *hsl,float*rgb);
void  hlPrintColor(const hlColor *color);
#endif

