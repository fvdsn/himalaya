#ifndef __HL_META_IMG_H__
#define __HL_META_IMG_H__
#include "hlImg"
typedef struct hl_meta_img hlMetaImg;
hlMetaImg *hlNewMetaImg(hlColor c, int sx, int sy);
void hlMetaImgSetDPM(hlMetaImg*m, float dpm);
void hlMetaImgSetWindow(hlMetaImg *m, int wx, int wy, int wsx, int wsy);
void hlMetaImgSetName(hlMetaImg *m, char *name);
void hlMetaImgSetFileName(hlMetaImg *m, char *filename);
char*hlMetaImgGetName(hlMetaImg *m);
char*hlMetaImgGetFileName(hlMetaImg*m);
void hlMetaImgGetWindowSize(hlMetaImg *m, int*wsx,int*wsy);
uint8_t*hlMetaImgGetWindowBuffer(hlMetaImg*m);
void hlMetaImgRender();

#endif
