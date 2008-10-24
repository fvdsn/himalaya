#include<stdint.h>

#ifndef __HL_COLOR_SPACE_H__
#define __HL_COLOR_SPACE_H__


typedef struct hl_cs{
	uint8_t	bpc;
	uint8_t	chan;
	uint8_t space;
}hlCS;

#define HL_MAX_CHANNELS 5
#define HL_MAX_BYTESPERCHANNEL 4 
#define HL_MAX_BYTEPERPIXEL ( HL_MAX_CHANNELS * HL_MAX_BYTESPERCHANNEL )

typedef enum hl_bpc{
	HL_8B  = 0,
	HL_16B = 1,
	HL_32B = 2
} HLBPC;

typedef enum hl_space{
	HL_RAW,
	HL_GRAY,
	HL_RGB,
	HL_CMY,
	HL_CMYK,
	HL_HSV,
	HL_LAB,
	HL_XYZ,
	HL_LUV  
} HLSPACE;

hlCS hlNewCS(HLBPC bitperchannel, HLSPACE space);                
hlCS hlCSCopy(hlCS colorspace);

HLBPC    hlCSGetBpc(hlCS colorspace);
HLSPACE  hlCSGetSpace(hlCS colorspace);
int      hlCSGetChan(hlCS colorspace);
int      hlCSGetBpp(hlCS colorspace); /* returns bytes per pixel */
int 	 hlCSEqual(hlCS a, hlCS b);

void hlCSSetBpc(hlCS* colorspace, HLBPC bitperchannel);
void hlCSSetSpace(hlCS* colorspace, HLSPACE space);
void hlCSSetChan(hlCS* colorspace, int channels);

void hlPrintCS(hlCS colorspace);

#endif


