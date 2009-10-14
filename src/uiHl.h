#ifndef __UI_HL_IMAGE_H__
#define __UI_HL_IMAGE_H__
#include "gui/uiCore.h"
#include "hl/hlImg.h"

typedef struct ui_hl_data{
	hlImg *img;
	hlState state;
	hlRaw *raw;
	float zoom;
}uiHlData;

uiEntity *uiHlNew(const char *name, hlImg *img, hlState s, hlRaw *raw);
void	uiHlSetImg(uiEntity *hl, hlImg *img);
void	uiHlSetState(uiEntity *hl, hlState s);
void	uiHLSetRenderSize(uiEntity *hl, int sx, int sy);
#endif

