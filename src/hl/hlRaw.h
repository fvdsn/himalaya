#include "hlColorSpace.h"
#include "hlColor.h"
#include <stdint.h>

#ifndef __HL_RAW_H__
#define __HL_RAW_H__

typedef struct hl_raw {
	hlCS cs;
	int sx;
	int sy;
	uint8_t *data;
} hlRaw;

hlRaw* hlNewRaw(hlCS cs, int sizex, int sizey); 
void   hlFreeRaw(hlRaw *raw);
void   hlPrintRaw(hlRaw *raw);

uint64_t  hlRawGetByteCount(hlRaw *raw);

void hlRawFill(hlRaw *raw, const hlColor *color);
void hlRawFlipY(hlRaw *raw);
void hlRawRandom(hlRaw *raw);
void hlRawZero(hlRaw *raw);
int  hlRawSizeX(hlRaw *raw);
int  hlRawSizeY(hlRaw *raw);
hlCS  hlRawCS(hlRaw *raw);
uint8_t *hlRawData(hlRaw *raw);

hlRaw* hlRawFromPng(char* path);
void hlRawToPng(hlRaw*r,char* path);

#endif

