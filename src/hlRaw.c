#include "hlRaw.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include "hlDebug.h"

int ilinit = 0;

long int random();

/* 	hlNewRaw(...) / hlFreeRaw(...) / etc... */
hlRaw* hlNewRaw(hlCS cs, int sizex, int sizey){
	hlRaw* raw = (hlRaw*)malloc(sizeof(hlRaw));
	raw->sx = sizex;
	raw->sy = sizey;
	raw->cs = cs;
	raw->data = (uint8_t*)malloc(sizex*sizey*hlCSGetBpp(cs));
	hl_debug_mem_alloc_various(sizex*sizey*hlCSGetBpp(cs));
	hl_debug_mem_alloc(HL_MEM_RAW);
	return raw;
}
void hlFreeRaw(hlRaw *raw){
	free(raw->data);
	hl_debug_mem_alloc_various(hlRawSizeX(raw)*hlRawSizeY(raw)*hlCSGetBpp(hlRawCS(raw)));
	free(raw);
	hl_debug_mem_free(HL_MEM_RAW);
	return;
}
uint64_t hlRawGetByteCount(hlRaw *raw){
	return raw->sx*raw->sy*hlCSGetBpp(raw->cs);
}
void hlRawFill(hlRaw *raw, const hlColor *col){
	int i = hlRawGetByteCount(raw);
	const int n = hlCSGetBpp(raw->cs);
	char* data = (char*)raw->data;
	assert(hlCSEqual(raw->cs,col->cs));
	while((i=i-n) >= 0){
		memcpy(data +i,(char*)col->color,n);
	}
	return;
}
void hlRawRandom(hlRaw *r){
	int i = hlRawGetByteCount(r);
	uint8_t *d = r->data;
	while(i--){
		d[i] = (uint8_t)(random()%255);
	}
	return;
}
void hlRawZero(hlRaw *raw){
	memset((char*)raw->data,0,hlRawGetByteCount(raw));
	return;
}
int hlRawSizeX(hlRaw *raw){
	return raw->sx;
}
int hlRawSizeY(hlRaw *raw){
	return raw->sy;
}
hlCS hlRawCS(hlRaw *raw){
	return raw->cs;
}
uint8_t *hlRawData(hlRaw *raw){
	return raw->data;
}
/* 		hlRawFlipY(...) / hlRawFlipX(...) 	*/
void memswitch(uint8_t *a, uint8_t *b, uint32_t n){
	uint8_t *tmp = (uint8_t*)malloc(n);
	memcpy((char*)tmp,(char*)a,n);
	memcpy((char*)a,(char*)b,n);
	memcpy((char*)b,(char*)tmp,n);
	free(tmp);
}
void hlRawFlipY(hlRaw *r){ 
	int i = r->sy;
	const int linesize = r->sx*hlCSGetBpp(r->cs);
	while(--i >= r->sy/2){
		memswitch(	&r->data[i*linesize],
				&r->data[(r->sy -i -1)*linesize],
				linesize);
	}
}

/* 		hlRawFromPng(...) / hlRawToPng(...) 	*/
hlRaw* hlRawFromPng(char *path){
	hlRaw * r;
	uint8_t *pngdata;
	ILuint img = 0;
	int sx = 0;
	int sy = 0;
	if(!ilinit){
		ilInit();
		iluInit();
		ilinit = 1;
	}
	ilGenImages(1,&img);
	ilBindImage(img);
	ilLoadImage(path);
	ilConvertImage(IL_RGBA,IL_UNSIGNED_BYTE);
	sx = ilGetInteger(IL_IMAGE_WIDTH);
	sy = ilGetInteger(IL_IMAGE_HEIGHT);
	r = hlNewRaw(hlNewCS(HL_8B,HL_RGB),sx,sy);
	pngdata = ilGetData();
	memcpy((char*)r->data, (char*)pngdata, hlRawGetByteCount(r));
	return r;
}
void hlRawToPng(hlRaw*r,char *path){
	ILuint img = 0;
	if(!ilinit){
		ilInit();
		iluInit();
		ilinit = 1;
	}
	assert(	hlCSGetBpc(r->cs) == HL_8B && 
		hlCSGetChan(r->cs) == 4 );
	ilGenImages(1,&img);
	ilBindImage(img);
	printf("coucou\n");
	hlRawFlipY(r);
	ilTexImage(r->sx,r->sy,1,4,IL_RGBA,IL_UNSIGNED_BYTE,r->data);
	ilEnable(IL_FILE_OVERWRITE);
	printf("coucou\n");
	ilSaveImage(path);
	hlRawFlipY(r);
}

/* 		hlPrintRaw(...) 			*/
void hlPrintRaw(hlRaw*raw){
	printf("<hlRaw>\n");
	if (raw){
		printf("   "); hlPrintCS(raw->cs);
		printf("   sx:%d\n",raw->sx);
		printf("   sy:%d\n",raw->sx);
		printf("   data:");
		if(raw->data)
			printf("OK\n");
		else
			printf("NULL\n");
	}
	else{
		printf("   NULL\n");
	}
	printf("</hlRaw>\n");
	return;
}

/*
int main(int argc, char **argv){
	hlRaw *r = hlRawFromPng("test.png");
	hlCS cs = hlNewCS(HL_8B,HL_RGB);
	hlColor c = hlNewColor(cs,255,0,64,0,255);
	hlRawToPng(r,"out.png");
	hlRawFill(r,&c);
	hlRawToPng(r,"out2.png");
	return 1;
}*/

