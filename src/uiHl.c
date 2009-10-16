#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "uiHl.h"
#include "gui/uiWidget.h"
#include <GL/gl.h>
#include <GL/glu.h>
float step = 0.1;
float radius = 32.0f;
float alpha = 1.0f;
float softness = 0.9f;
float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};

#define MARGIN 32
static int uiHlClick(uiEntity *self, int button, int down, float x, float y, float p){
	uiHlData *hd = (uiHlData*)self->data;
	if(button == 0){
		if(down == UI_KEY_DOWN){
			hd->painting = 1;
			/*TODO that's hackish*/
			hd->lpx = -1000000;
			hd->lpy = -1000000;
		}else{
			hd->painting = 0;
		}
	}
	return 0;
}
	
static int uiHlMotion(uiEntity *self, float x, float y, float p){
	uiHlData *hd = (uiHlData*)self->data;
	hlCS cs = hlNewCS(HL_8B,HL_RGB);
	hlColor col = hlNewColor(cs,	color[0],
					color[1],
					color[2],0,
					color[3]);
	hlOp *op;
	uiEntityMousePos(self,&x,&y,NULL);
	y = hlRawSizeY(hd->raw) - y;

	if(hd->painting){
		if(sqrtf((x-hd->lpx)*(x-hd->lpx) + (y-hd->lpy)*(y-hd->lpy)) >
				step*radius){
			hd->lpx = x;
			hd->lpy = y;
			printf("painting\n");
			op = hlNewOp(HL_DRAW_CIRCLE);
			hlOpSetAllValue(op,"pos_center",x,y);
			hlOpSetAllValue(op,"radius_in",radius*(1.0f-softness));
			hlOpSetAllValue(op,"radius_out",radius);
			hlOpSetAllValue(op,"alpha",alpha);
			hlOpSetAllColor(op,"fill_color",col);
			hlImgPushOp(hd->img,op);
			hd->uptodate = 0;
		}
	}
	return 0;
}
static void uiHlDraw(uiEntity *self){
	float x = 0;
	float y = 0;
	float z = 0;
	float sx = 0;
	float sy = 0;
	uiHlData*hd = (uiHlData*)self->data;
	if(!hd->raw){
		return;
	}
	if(hd->img && !hd->uptodate){
		hlImgRenderToRaw(hd->img,hd->raw,0,0,0,0);
		hd->uptodate=1;
	}
	sx = hlRawSizeX(hd->raw);
	sy = hlRawSizeY(hd->raw);
	glColor4f(0,0,0,0.2);
	uiRectDraw(x-MARGIN+5,y-MARGIN-5,z,sx+2*MARGIN,sy+2*MARGIN);
	glColor4f(0.5,0.5,0.5,1);
	uiRectDraw(x-MARGIN,y-MARGIN,z,sx+2*MARGIN,sy+2*MARGIN);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glColor4f(1,1,1,1);
	glTexImage2D(	GL_TEXTURE_RECTANGLE_ARB,
			0,
			GL_RGBA,
			hlRawSizeX(hd->raw),
			hlRawSizeY(hd->raw),
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			(char*)hlRawData(hd->raw) );
	uiRectDraw(x,y,z,sx,sy);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glEnable(GL_BLEND);
}
uiEntity *uiHlNew(const char *name, hlImg *img, hlState s,hlRaw *raw){
	uiEntity *h = uiEntityNew(name,UI_ENT_HL);
	uiHlData *hd = (uiHlData*)malloc(sizeof(uiHlData));
	memset((char*)hd,0,sizeof(uiHlData));
	h->data = hd;
	h->draw = uiHlDraw;
	h->click = uiHlClick;
	h->motion = uiHlMotion;
	hd->img = img;
	hd->state = s;
	hd->raw = raw;
	hd->zoom = 1.0f;
	if(raw){
		uiEntitySetSize(h,hlRawSizeX(raw)+2*MARGIN,hlRawSizeY(raw)+2*MARGIN);
	}
	return h;
}
void	uiHlSetImg(uiEntity *hl, hlImg *img){
	/*TODO assert correct entity type */
	((uiHlData*)hl->data)->img = img;
}
void	uiHlSetState(uiEntity *hl, hlState s){
	/*TODO assert correct entity type */
	((uiHlData*)hl->data)->state = s;
}
void	uiHlRender(uiEntity *hl){
	/*TODO assert correct entity type */
	((uiHlData*)hl->data)->uptodate = 0;
}


	
