#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "uiHl.h"
#include "gui/uiWidget.h"
#include <GL/gl.h>
#include <GL/glu.h>

#define BUTTON_MOVE  1
#define BUTTON_PAN  2

/*tool*/
float step = 0.2;
float radius = 32.0f;
float alpha = 1.0f;
float softness = 0.9f;
float randomness = 0.0f;
float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};

/*viewport size in hl coordinates*/
float hlpx = 0.0;
float hlpy = 0.0;
float hlsx = 0.0;
float hlsy = 0.0;
int   hlzoomlevel = 0;
int   hlopcount = 0;


static int uiHlMotion(uiEntity *self, float x, float y, float p){
	uiHlData *hd = (uiHlData*)self->data;
	hlColor col = hlNewColor(	hd->cs,
					color[0],
					color[1],
					color[2],0,
					color[3]);
	hlOp *op;
	uiEntityMousePos(self,&x,&y,NULL);
	if(uiStateKey('z') || uiStateKey('x')){
		return 0;
	}else if(uiStateMouse(BUTTON_MOVE) == UI_KEY_DOWN){
		uiStateMouseDelta(&x,&y,NULL);
		self->posx += x;
		self->posy += y;
		return 0;
	}else if(uiStateMouse(BUTTON_PAN) == UI_KEY_DOWN){
		uiStateMouseDelta(&x,&y,NULL);
		hd->dx += x;
		hd->dy += y;
		hd->uptodate = 0;
		return 0;
	}
	if(hd->painting){
		/*position of click in hl zoom coordinates */
		float py =  (hd->sy - (y - hd->dy))*powf(2,hd->zoomlevel); 	
		float px =  (x - hd->dx)*powf(2,hd->zoomlevel);
		/*vector from last click in hl zoom coordinates */
		float dpx = (px-hd->lpx);	
		float dpy = (py-hd->lpy);
		float dplength = sqrtf(dpx*dpx + dpy*dpy);
		/* we prevent drawing if the stepsize is below 0.25 pixel in
		 * screenspace so that the user doesn't introduce billions of
		 * operations in one click */	
		if(step*radius*powf(2,-hd->zoomlevel) < 0.25){
			printf("WARNING : step size is too small at this zoomlevel");
			return 0;
		}
		/* we draw the first brush disregarding the step between the
		 * last one */
		if(hd->starting){
			hd->lpx = px;
			hd->lpy = py;
			printf("Paint start at [%f,%f]\n", px, py);
			op = hlNewOp(HL_DRAW_CIRCLE);
			hlOpSetAllValue(op,"pos_center",px,py);
			hlOpSetAllValue(op,"radius_in",radius*(1.0f-softness));
			hlOpSetAllValue(op,"radius_out",radius);
			hlOpSetAllValue(op,"alpha",alpha);
			hlOpSetAllColor(op,"fill_color",col);
			hlImgPushOp(hd->img,op);
			hlopcount++;
			hd->uptodate = 0;
			hd->starting = 0;
			return 0;
		}
		/* we draw one brush every step between the two clicks */
		while(dplength > step*radius){
			float rpx = (randomness*radius)*(random()/(float)RAND_MAX - 0.5f);
			float rpy = (randomness*radius)*(random()/(float)RAND_MAX - 0.5f);
			float dstepx = dpx / dplength * step * radius;
			float dstepy = dpy / dplength * step * radius;
			hd->lpx += dstepx;
			hd->lpy += dstepy;
			dpx -= dstepx;
			dpy -= dstepy;
			dplength = sqrtf(dpx*dpx + dpy*dpy);
			printf("Paint continues at [%f,%f]\n", hd->lpx, hd->lpy);
			op = hlNewOp(HL_DRAW_CIRCLE);
			hlOpSetAllValue(op,"pos_center",hd->lpx +rpx,hd->lpy +rpy);
			hlOpSetAllValue(op,"radius_in",radius*(1.0f-softness));
			hlOpSetAllValue(op,"radius_out",radius);
			hlOpSetAllValue(op,"alpha",alpha);
			hlOpSetAllColor(op,"fill_color",col);
			hlImgPushOp(hd->img,op);
			hlopcount++;
			hd->uptodate = 0;
		}
	}
	return UI_DONE;
}
static int uiHlClick(uiEntity *self, int button, int down, float x, float y, float p){
	uiHlData *hd = (uiHlData*)self->data;
	if(button == 0){
		if(down == UI_KEY_DOWN){
			hd->painting = 1;
			if(uiStateKey('a')){
				hd->starting = 0;
			}
			/*TODO that's hackish*/
			uiHlMotion(self,x,y,p);
		}else{
			uiHlPushState(self);
			hd->painting = 0;	
			hd->starting = 1;	/* next click will be beginning of new stroke */
		}
	}else if(down == UI_KEY_UP){
		if(uiStateKey('z')){
			uiHlSetZoomLevel(self, hd->zoomlevel+ 1);
		}else if(uiStateKey('x')){
			uiHlSetZoomLevel(self,hd->zoomlevel-1);
		}else if(uiStateKey('u')){
			uiHlUndo(self);
		}else if(uiStateKey('r')){
			uiHlRedo(self);
		}
	}
	printf("%d\n",uiStateKey('z'));
	return UI_DONE;
}
static void uiHlDraw(uiEntity *self){
	uiHlData*hd = (uiHlData*)self->data;
	if(!hd->img){
		return;
	}
	/*size of the entity has changed, resize the viewport*/
	if(!hd->raw || hd->sx != (int)self->sizex || hd->sy != (int)self->sizey){
		printf("viewport resize from [%d,%d] to [%d.%d]\n",hd->sx,hd->sy,(int)self->sizex,(int)self->sizey);
		hd->sx = (int)self->sizex;
		hd->sy = (int)self->sizey;
		hlFreeRaw(hd->raw);
		hd->raw = hlNewRaw(hd->cs,hd->sx,hd->sy);
		hd->uptodate = 0;
	}
	/* viewport needs to be redrawn */
	if(!hd->uptodate){
		hlImgRenderToRaw(hd->img,hd->raw,0,-hd->dx,hd->dy,hd->zoomlevel);
		hd->uptodate=1;
		/*update viewport statistics*/
		hlpx = -hd->dx*powf(2,hd->zoomlevel);
		hlpy = hd->dy*powf(2,hd->zoomlevel);
		hlsx = hd->sx*powf(2,hd->zoomlevel);
		hlsy = hd->sy*powf(2,hd->zoomlevel);
	}
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glColor4f(1,1,1,1);
	glTexImage2D(	GL_TEXTURE_RECTANGLE_ARB,
			0,
			GL_RGBA,
			hd->sx,
			hd->sy,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			(char*)hlRawData(hd->raw) );
	uiRectDraw(0,0,0,hd->sx,hd->sy);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glEnable(GL_BLEND);
}
uiEntity *uiHlNew(const char *name, hlImg *img, hlState s,int sx, int sy){
	uiEntity *h = uiEntityNew(name,UI_ENT_HL);
	uiHlData *hd = (uiHlData*)malloc(sizeof(uiHlData));
	memset((char*)hd,0,sizeof(uiHlData));
	h->data = hd;
	h->draw = uiHlDraw;
	h->click = uiHlClick;
	h->motion = uiHlMotion;
	hd->sx = sx;
	hd->sy = sy;
	hd->starting = 1;
	uiEntitySetSize(h,sx,sy);
	uiHlSetImg(h,img,s);
	hd->raw = hlNewRaw(hd->cs,sx,sy);
	return h;
}
void	uiHlSetImg(uiEntity *hl, hlImg *img, hlState s){
	uiHlData *hd = (uiHlData*)hl->data;
	/*TODO assert correct entity type */
	hd->img = img;
	hd->state = s;
	hd->cs = hlImgCS(img);
	uiHlSetZoomLevel(hl,0);
}
void	uiHlSetState(uiEntity *hl, hlState s){
	/*TODO assert correct entity type */
	((uiHlData*)hl->data)->state = s;
	((uiHlData*)hl->data)->uptodate = 0;
}
void	uiHlRender(uiEntity *hl){
	/*TODO assert correct entity type */
	((uiHlData*)hl->data)->uptodate = 0;
}
void	uiHlSetZoomLevel(uiEntity *hl, int zoomlevel){
	uiHlData *hd = (uiHlData*)hl->data;
	if(zoomlevel >= 0 && zoomlevel < 32){
		hd->zoomlevel = zoomlevel;
		hlzoomlevel = zoomlevel;
		hd->uptodate = 0;
	}
}
void uiHlBaseState(uiEntity *hl){
	uiHlData *hd = (uiHlData*)hl->data;
	if(hd->hist_cur_index == 0
			&&	hd->hist_last_index == 0
			&&	hd->history[0] == 0){
		hd->history[0] = hlImgStateSave(hd->img);
	}
}
void uiHlPushState(uiEntity *hl){
	uiHlData *hd = (uiHlData*)hl->data;
	if(hd->hist_cur_index < MAX_UNDO_LEVEL){
		while(hd->hist_last_index > hd->hist_cur_index){
			hlImgStateRem(hd->img,hd->history[hd->hist_last_index]);
			hd->hist_last_index--;
		}
		hd->hist_cur_index++;
		hd->hist_last_index++;
		hd->history[hd->hist_cur_index] = hlImgStateSave(hd->img);
		printf("Current undo level : %d \n",hd->hist_cur_index);
	}
}
void uiHlUndo(uiEntity *hl){
	uiHlData *hd = (uiHlData*)hl->data;
	if(hd->hist_cur_index >0){
		hd->hist_cur_index--;
		hlImgStateLoad(hd->img,hd->history[hd->hist_cur_index]);
		hd->uptodate = 0;
	}
	printf("Current undo level : %d \n",hd->hist_cur_index);
}
		
void uiHlRedo(uiEntity *hl){
	uiHlData *hd = (uiHlData*)hl->data;
	if(hd->hist_cur_index < hd->hist_last_index){
		hd->hist_cur_index++;
		hlImgStateLoad(hd->img,hd->history[hd->hist_cur_index]);
		hd->uptodate = 0;
	}
	printf("Current undo level : %d \n",hd->hist_cur_index);
}




	
