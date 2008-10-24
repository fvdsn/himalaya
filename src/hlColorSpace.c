#include "hlColorSpace.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

/* 	hlNewCS(...)/hlCSCopy(...) 		*/
int  hl_default_space_chan(HLSPACE space){
	switch(space){
		case HL_RAW  : return 1 ; 
		case HL_GRAY : return 2 ; 
		case HL_RGB  : return 4 ;
		case HL_CMY  : return 4 ;
		case HL_CMYK : return 5 ;
		case HL_HSV  : return 4 ;
		case HL_LAB  : return 4 ;
		case HL_XYZ  : return 4 ;
		case HL_LUV  : return 4 ;
		default : {
			assert(0 && "unknown space");
			return 1;
		}
	}
}
hlCS hlNewCS(HLBPC bitperchannel, HLSPACE space){
	hlCS colorspace;
	hlCSSetBpc(&colorspace, bitperchannel);
	hlCSSetChan(&colorspace, hl_default_space_chan(space));
	hlCSSetSpace(&colorspace, space);
	return colorspace;
}
hlCS hlCSCopy(hlCS colorspace){
	return colorspace;
}

/* 	hlCSGet___(...)				*/
HLBPC   hlCSGetBpc(hlCS cs){
	return cs.bpc;
}
int     hlCSGetChan(hlCS cs){
	return cs.chan;
}
HLSPACE hlCSGetSpace(hlCS cs){
	return cs.space;
}
int     hlCSGetBpp(hlCS cs){
	int chan = hlCSGetChan(cs);
	switch (hlCSGetBpc(cs)){
		case HL_8B  : return chan;
		case HL_16B : return 2*chan;
		case HL_32B : return 4*chan;
	}
	return 0;
}
int  hlCSEqual(hlCS a, hlCS b){
	if ( 	a.bpc   == b.bpc &&
		a.chan  == b.chan &&
		a.space == b.space )
		return 1;
	else
		return 0;
}

/*	hlCSSet___(...) 			*/
void hlCSSetBpc(hlCS* cs, HLBPC bitperchannel){
	cs->bpc = bitperchannel;
	return;
}
void hlCSSetChan(hlCS* cs, int channels){
	assert(channels >= 1 && channels <= HL_MAX_CHANNELS);
	cs->chan = channels;
	return;
}
void hlCSSetSpace(hlCS* cs, HLSPACE space){
	cs->space = space;
}

/*	hlPrintCS(...) 				*/
void hlPrintCS(hlCS cs){
	printf("<hlCS> ");
	switch(hlCSGetBpc(cs)){
		case HL_8B  : { printf("HL_INT8, ");  break; }
		case HL_16B : { printf("HL_INT16, "); break; }
		case HL_32B : { printf("HL_FLT32, "); break; }
		default : printf("ERROR, ");
	}
	printf("%d, ",hlCSGetChan(cs));
	switch(hlCSGetSpace(cs)){
		case HL_RAW  : { printf("HL_RAW\n");  break; }
		case HL_GRAY : { printf("HL_GRAY\n"); break; }
		case HL_RGB  : { printf("HL_RGB\n");  break; }
		case HL_CMY  : { printf("HL_CMY\n");  break; }
		case HL_CMYK : { printf("HL_CMYK\n"); break; }
		case HL_HSV  : { printf("HL_HSV\n");  break; }
		case HL_LAB  : { printf("HL_LAB\n");  break; }
		case HL_XYZ  : { printf("HL_XYZ\n");  break; }
		case HL_LUV  : { printf("HL_LUV\n");  break; }
		default : printf("ERROR\n");
	}
	printf("</hlCS>\n");
	return;
}

