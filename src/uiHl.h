#ifndef __UI_HL_IMAGE_H__
#define __UI_HL_IMAGE_H__
#include "gui/uiCore.h"
#include "hl/hlImg.h"

#define MAX_UNDO_LEVEL 1024

typedef struct ui_hl_data{
	hlImg *img;
	hlState state;
	hlRaw *raw;
	hlCS cs;

	int uptodate;
	int zoomlevel;
	int sx;		/* the size of the viewport */
	int sy;
	int dx;		/* position of the image inside the viewport */
	int dy;

	/*painting*/
	int painting;	/*1 if paint tool is active */
	int starting;	/*1 if this is the beginning of a new brush stroke */
	float lpx;	/*position of last brush drawn in hl coordinates*/
	float lpy;

	hlState history[MAX_UNDO_LEVEL];
	int hist_cur_index;
	int hist_last_index;
}uiHlData;

uiEntity *uiHlNew(const char *name,hlImg *img, hlState s, int sx, int sy);
void	uiHlSetImg(uiEntity *hl, hlImg *img, hlState s);
void	uiHlSetState(uiEntity *hl, hlState s);
void	uiHlRender(uiEntity *hl);
void	uiHlSetZoomLevel(uiEntity *hl, int zoomlevel);
void uiHlBaseState(uiEntity *hl);
void uiHlPushState(uiEntity *hl);
void uiHlUndo(uiEntity *hl);
void uiHlRedo(uiEntity *hl);
#endif

