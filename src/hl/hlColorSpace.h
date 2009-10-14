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
	HL_32B = 1
}hlBpc;

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
}hlSpace;

hlCS hlNewCS(hlBpc bitperchannel, hlSpace space);                
hlCS hlCSCopy(hlCS colorspace);

hlBpc    hlCSGetBpc(hlCS colorspace);
hlSpace  hlCSGetSpace(hlCS colorspace);
int      hlCSGetChan(hlCS colorspace);
int      hlCSGetBpp(hlCS colorspace); /* returns bytes per pixel */
int 	 hlCSEqual(hlCS a, hlCS b);

void hlCSSetBpc(hlCS* colorspace, hlBpc bitperchannel);
void hlCSSetSpace(hlCS* colorspace, hlSpace space);
void hlCSSetChan(hlCS* colorspace, int channels);

void hlPrintCS(hlCS colorspace);

#endif


