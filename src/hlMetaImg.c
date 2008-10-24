#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hlMetaImg.h"
#define HL_MAX_STRING_LENGTH 1024
#define HL_DEFAULT_DPM 100
#define HL_DEFAULT_VIEWPORT 1000
struct hl_meta_img{
	int mx; /*size in m */
	int my;
	hlRegion image_region;
	hlRegion window_region;
	hlState state;
	char name[HL_MAX_STRING_LENGTH];
	char file[HL_MAX_STRING_LENGTH];
	hlImg *img;
	hlRaw *buffer;
}
hlMetaImg *hlNewMetaImg(hlColor c, int sx, int sy){
	hlMetaImg *mimg = (hlMetaImg*)malloc(sizeof(hlMetaImg));
	mimg->mx = sx/HL_DEFAULT_DPCM;
	mimg->my = sy/HL_DEFAULT_DPCM;
	mimg->image_region = hlNewRegion(0,0,sx,sy,0);
	mimg->window_region = hlNewRegion(0,0,HL_DEFAULT_VIEWPORT,HL_DEFAULT_VIEWPORT,0);
	mimg->img = hlNewImg(&c,sx,sy);
	mimg->buffer = hlNewRaw(hlColorCS(&c),mimg->wsx,mimg->wsy);
	hlRawFill(mimg->buffer,&c);
	return mimg;
}
void hlMetaImgSetDPM(hlMetaImg*m,float dpm){
	if(dpm > 0.0){
		m->mx = m->image_region.sx / dpm;
		m->my = m->image_region.sy / dpm;
	}
	return;
}
void hlMetaImgSetWindow(hlMetaImg *m, int wx, int wy, int wsx, int wsy){
	if(wsx > 0){ m->wsx = wsx;}
	else{ m->wsx = 1; }
	if(wsy > 0){ m->wsy = wsy;}
	else{ m->wsy = 1; }
	m->wx = wx;
	m->wy = wy;
	return;
}
void hlMetaImgSetName(hlMetaImg *m, char *name){
	strncpy(m->name,name,HL_MAX_STRING_LENGTH);
	m->name[HL_MAX_STRING_LENGTH - 1] = 0;
	return;
}
void hlMetaImgSetFileName(hlMetaImg *m,char *filename){
	strncpy(m->file,filename,HL_MAX_STRING_LENGTH);
	m->file[HL_MAX_STRING_LENGTH - 1] = 0;
	return;
}
char *hlMetaImgGetName(hlMetaImg*m){
	return m->name;
}
char *hlMetaImgGetFileName(hlMetaImg*m){
	return m->filename;
}
void hlMetaImgGetWindowSize(hlMetaImg*m,int*wsx,int*wsy){
	if(wsx){*wsx = m->wsx;}
	if(wsy){*wsy = m->wsy;}
	return;
}
uint8_t*hlMetaImgGetWindowBuffer(hlMetaImg*m){
	return hlRawData(m->buffer);
}


