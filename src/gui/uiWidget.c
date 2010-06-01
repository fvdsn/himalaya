#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cairo.h>
#include <math.h>
#include "uiCore.h"
#include "uiWidget.h"

uiEntity *uiScreenNew(const char *name){
	uiEntity *s = uiEntityNew(name,UI_ENT_SCREEN);
	uiEntitySetSize(s,10000,10000);
	uiEntitySetPos(s,0,0);
	return s;
}
static void uiButtonDraw(uiEntity *self){
	float *color = uiWindowGetColor(UI_ITEM_COLOR,UI_NORMAL_COLOR);
	float x = 0; 
	float y = 0;
	float z = 0;
	glColor4f(0,0,0,0.2);
	uiRectDraw(x+3,y-3,z-0.1,self->sizex,self->sizey);
	if(self->mouseover){
		glColor4f(1,0.2,0,1);
	}else{
		glColor4f(color[0],color[1],color[2],color[3]);
	}
	uiRectDraw(x,y,z,self->sizex,self->sizey);
	uiStringDraw(self->string,x,y,z,self->sizex,self->sizey);
}
static int uiButtonClick(uiEntity *self,int button, int down, float x, float y, float pressure){
	uiButtonData *bd = (uiButtonData*)self->data;
	//printf("click!:%s\tbutton:%d\tdown:%d\n",self->name,button,down);
	if(bd->click && button == 0 && down == UI_KEY_UP){
		bd->click(self,bd->id);
	}
	return 0;
}
uiEntity *uiButtonNew(const char *name,
		int id,
		void(*click)(uiEntity *self, int id)){
	uiButtonData *bd = (uiButtonData*)malloc(sizeof(uiButtonData));
	uiEntity *b = uiEntityNew(name,UI_ENT_BUTTON);
	uiEntitySetSize(b,UI_BUTTON_WIDTH,16);
	bd->id = id;
	bd->click = click;
	b->data = bd;
	b->draw = uiButtonDraw;
	b->click = uiButtonClick;
	b->string = uiStringNew(name,10,UI_BUTTON_WIDTH,16,5,11);
	return b;
}
static void uiLabelDraw(uiEntity *self){
	uiStringDraw(self->string,0,0,0,self->sizex,self->sizey);	
}
uiEntity *uiLabelNew(const char *name, const char *text,float font_size){
	uiEntity *l = uiEntityNew(name,UI_ENT_LABEL);
	l->string = uiStringNew(text, font_size,50,20,5,11);
	l->draw = uiLabelDraw;
	return l;
}
static void uiPanelDraw(uiEntity *self){
	float *color = uiWindowGetColor(UI_PANEL_COLOR,UI_NORMAL_COLOR);
	float x = 0;
	float y = 0;
	float z = 0;
	glColor4f(0,0,0,0.3);
	uiRectDraw(x+3,y-3,z-0.1,self->sizex,self->sizey);
	glColor4f(color[0],color[1],color[2],color[3]);
	uiRectDraw(x,y,z,self->sizex,self->sizey);
}
static int uiPanelMotion(uiEntity *self,float x, float y, float p){
	float dx,dy;
	if(uiStateMouse(0)){
		uiStateMouseDelta(&dx,&dy,NULL);
		self->posx += dx;
		self->posy += dy;
	}
	return 0;
}
uiEntity *uiPanelNew(const char *name){
	uiEntity *p = uiEntityNew(name,UI_ENT_PANEL);
	uiEntitySetSize(p,UI_PANEL_WIDTH,UI_PANEL_HEIGHT);
	p->draw = uiPanelDraw;
	p->motion = uiPanelMotion;
	return p;
}
void	 uiSliderSetValue(uiEntity *slider, float value){
	uiSliderData *sd = (uiSliderData*)slider->data;
	if(value >= sd->max_value){
		value = sd->max_value;
	}else if(value < sd->min_value){
		value = sd->min_value;
	}
	if(sd->slide){
		sd->slide(slider,value,sd->id);
		if(sd->base == 0.0f){
			sd->value = value;
		}else{
			sd->value = logf(value)/logf(sd->base);
			sd->exp_value = value;
		}
	}else if(sd->dest_value){
		if(sd->base == 0.0f){
			sd->value = value;
			*(sd->dest_value) = sd->value;
		}else{
			sd->value = logf(value)/logf(sd->base);
			sd->exp_value = value;
			*(sd->dest_value) = sd->exp_value;
		}
	}
}
float	 uiSliderGetValue(uiEntity *slider){
	uiSliderData *sd = (uiSliderData*)slider->data;
	if (sd->base == 0.0f){
		return sd->value;
	}else{
		return sd->exp_value;
	}
}
static int uiSliderMotion(uiEntity *self,float x, float y, float p){
	float dx,v,us;
	uiSliderData *sd = (uiSliderData*)self->data;
	us = sd->update_speed;
	if(uiStateMod(UI_SHIFT) == UI_KEY_DOWN){
		us *= 0.1;
	}else if(uiStateMod(UI_ALT) == UI_KEY_DOWN){
		us *= 10;
	}
	if(uiStateMouse(UI_MOUSE_BUTTON_1)){
		uiStateMouseDelta(&dx,NULL,NULL);
		v = sd->value + dx*us;
		if(v >= sd->max_value){
			sd->value = sd->max_value;
		}else if (v < sd->min_value){
			sd->value = sd->min_value;
		}else{
			sd->value = v;
		}
		if(dx != 0.0){
			if (sd->slide){
				sd->slide(self,sd->value,sd->id);
			}else if(sd->dest_value){
				*(sd->dest_value) = sd->value;
			}
		}
	}
	return 0;
}
static int uiSliderExpMotion(uiEntity *self,float x, float y, float p){
	float dx,v, ev,us;
	uiSliderData *sd = (uiSliderData*)self->data;
	us = sd->update_speed;
	if(uiStateMod(UI_SHIFT) == UI_KEY_DOWN){
		us *= 0.1;
	}else if(uiStateMod(UI_ALT) == UI_KEY_DOWN){
		us *= 10;
	}
	if(uiStateMouse(UI_MOUSE_BUTTON_1)){
		uiStateMouseDelta(&dx,NULL,NULL);
		v = sd->value + dx*us;
		ev = powf(sd->base,v);
		if(ev > sd->max_value){
			sd->value = logf(sd->max_value)/logf(sd->base);
			sd->exp_value = sd->max_value;
		}else if (ev < sd->min_value){
			sd->value = logf(sd->min_value)/logf(sd->base);
			sd->exp_value = sd->min_value;
		}else{
			sd->value = v;
			sd->exp_value = ev;
		}
		if(dx != 0.0){
			if (sd->slide){
				sd->slide(self,sd->exp_value,sd->id);
			}else if(sd->dest_value){
				*(sd->dest_value) = sd->exp_value;
			}
		}
	}
	return 0;
}
static void uiSliderDraw(uiEntity *self){
	float *color = uiWindowGetColor(UI_ITEM_COLOR,UI_NORMAL_COLOR);
	float bgcol[4] = {0.0,0.0,0.0,0.2};
	float barcol[4] = {0.3,0.3,0.3,1.0};
	float active[4] = {1.0,0.2,0,1.0};
	float x = 0;
	float y = 0;
	float z = 0;
	float margin = 3.0;

	uiSliderData *sd = (uiSliderData*)self->data;
	glColor4f(0,0,0,0.2);
	uiRectDraw(x+3,y-3,z-0.1,self->sizex,self->sizey);
	glColor4f(color[0],color[1],color[2],color[3]);
	uiRectDraw(x,y,z,self->sizex,self->sizey);
	glColor4f(bgcol[0],bgcol[1],bgcol[2],bgcol[3]);
	uiRectDraw(	x + margin, y + margin,z,
			self->sizex - 2*margin ,
			self->sizey - 2*margin	);
	if(self->mouseover){
		glColor4f(active[0],active[1],active[2],active[3]);
	}else{
		glColor4f(barcol[0],barcol[1],barcol[2],barcol[3]);
	}
	if(sd->base == 0.0f){
		uiRectDraw(	x + margin, y + margin,z,
			(self->sizex - 2*margin)
				*(sd->value/(sd->max_value-sd->min_value)) ,
			self->sizey - 2*margin	);
		snprintf(uiStringWrite(self->string),UI_TEXT_LENGTH,"%s : %.2f",
				self->name,sd->value);
	}else{
		uiRectDraw(	x + margin, y + margin,z,
			(self->sizex - 2*margin)
				*(sd->exp_value/(sd->max_value-sd->min_value)) ,
			self->sizey - 2*margin	);
		snprintf(uiStringWrite(self->string),UI_TEXT_LENGTH,"%s : %.2f",
					self->name,sd->exp_value);
	}
	uiStringDraw(self->string,x,y,z,self->sizex,self->sizey);
}

uiEntity *uiSliderNew(const char *name,
		int id,
		float min_value,
		float max_value,
		float update_speed,
		float base,
		float start_value,
		float *dest_value,
		void(*slide)(uiEntity *self, float value, int id)){
	uiSliderData *sd = (uiSliderData*)malloc(sizeof(uiSliderData));
	uiEntity *b = uiEntityNew(name,UI_ENT_SLIDER);
	uiEntitySetSize(b,UI_BUTTON_WIDTH,UI_BUTTON_HEIGHT);
	sd->id = id;
	sd->slide = slide;
	sd->min_value = min_value;
	sd->max_value = max_value;
	sd->update_speed = update_speed;
	sd->dest_value = dest_value;
	b->data = sd;
	b->draw = uiSliderDraw;
	b->string = uiStringNew(name,10,UI_BUTTON_WIDTH,UI_BUTTON_HEIGHT,5,11);
	if(base == 0.0f || base == 1.0f){
		b->motion = uiSliderMotion;
		sd->base = 0.0f;
		sd->exp_value = 0.0f;
		if(dest_value){
			*dest_value = start_value;
		}
		sd->value = start_value;
		if(sd->slide){
			sd->slide(b,sd->value,sd->id);
		}
	}else{
		b->motion = uiSliderExpMotion;
		sd->base = base;
		if(dest_value){
			sd->exp_value = *dest_value;
		}else{
			sd->exp_value = start_value;
		}
		sd->value = logf(sd->exp_value)/logf(sd->base);
		if(sd->slide){
			sd->slide(b,sd->exp_value,sd->id);
		}
	}
	return b;
}
static void uiDisplayFloatDraw(uiEntity *self){
	float *value = (float*)self->data;
	glColor4f(0.6,0.6,0.6,0.5);
	uiRectDraw(0,0,0,self->sizex,self->sizey);
	if(value){
		snprintf(uiStringWrite(self->string),UI_TEXT_LENGTH,"%s : %.2f",
			self->name,*value);
	}
	uiStringDraw(self->string,0,0,1,self->sizex,self->sizey);
}
static void uiDisplayIntDraw(uiEntity *self){
	int *value = (int*)self->data;
	glColor4f(0.6,0.6,0.6,0.5);
	uiRectDraw(0,0,0,self->sizex,self->sizey);
	if(value){
		snprintf(uiStringWrite(self->string),UI_TEXT_LENGTH,"%s : %d",
			self->name,*value);
	}
	uiStringDraw(self->string,0,0,1,self->sizex,self->sizey);
}
uiEntity *uiDisplayFloatNew(const char *name, float *display_value){
	uiEntity *d = uiEntityNew(name,UI_ENT_DISPLAY);
	uiEntitySetSize(d,UI_BUTTON_WIDTH,UI_BUTTON_HEIGHT);
	d->data = display_value;
	d->string = uiStringNew(name,10,UI_BUTTON_WIDTH,UI_BUTTON_HEIGHT,5,11);
	d->draw = uiDisplayFloatDraw;
	return d;
}
uiEntity *uiDisplayIntNew(const char *name, int *display_value){
	uiEntity *d = uiEntityNew(name,UI_ENT_DISPLAY);
	uiEntitySetSize(d,UI_BUTTON_WIDTH,UI_BUTTON_HEIGHT);
	d->data = display_value;
	d->string = uiStringNew(name,10,UI_BUTTON_WIDTH,UI_BUTTON_HEIGHT,5,11);
	d->draw = uiDisplayIntDraw;
	return d;
}
static void uiColorDraw(uiEntity *self){
	float *color = (float*)self->data;
	float x = 0;
	float y = 0;
	float z = 0;
	glColor4f(color[0],color[1],color[2],color[3]);
	uiRectDraw(x,y,z,self->sizex/2,self->sizey/2);
	uiRectDraw(	x+self->sizex/2, y+self->sizey/2,z,
			self->sizex/2,	self->sizey/2);
	glColor4f(color[0],color[1],color[2],1.0);
	uiRectDraw(x+self->sizex/2,y,z,self->sizex/2,self->sizey/2);
	uiRectDraw(x,y+self->sizey/2,z,self->sizex/2,self->sizey/2);
}

uiEntity *uiColorNew(const char *name, float *color){
	uiEntity *c = uiEntityNew(name,UI_ENT_COLOR);
	uiEntitySetSize(c,32,32);
	c->data = color;
	c->draw = uiColorDraw;
	return c;
}
#define SLIDER_WIDTH 10
#define SLIDER_MARGIN 2
#define SLIDER_Z 10
static void uiRegionDraw(uiEntity *self){
	glColor4f(0.45,0.45,0.45,1);
	uiRectDraw(0,0,0,self->sizex,self->sizey);
	/*vertical slider */
	glEnable(GL_DEPTH_TEST);
	if(self->dy < 0 || self->dy + self->inner_sizey > self->sizey){
		/*we need to draw a slider */
		/*draw the slider background */
		float px = self->sizex - SLIDER_WIDTH - SLIDER_MARGIN;
		float py = SLIDER_MARGIN;
		float sy = self->sizey - (2*SLIDER_MARGIN);
		if(sy <0){ sy = 0;}
		glColor4f(0.4,0.4,0.4,1);
		uiRectDraw(px,py,SLIDER_Z,SLIDER_WIDTH,sy);
		/*draw the slider handle */
		if(self->inner_sizey > 0){
			float handle_sy = sy * self->sizey / self->inner_sizey;
			float handle_py  = self->dy *self->sizey / self->inner_sizey;
			glColor4f(0.5,0.5,0.5,1);
			uiRectDraw(px,handle_py+SLIDER_MARGIN,SLIDER_Z+0.1,SLIDER_WIDTH,handle_sy);
		}
	}
	/*horizontal slider */
	if(self->dx < 0 || self->dx + self->inner_sizex > self->sizex){
		/*we need to draw a slider */
		/*draw the slider background */
		float py = SLIDER_MARGIN;
		float px = SLIDER_MARGIN;
		float sx = self->sizex - (2*SLIDER_MARGIN);
		if(sx <0){ sx = 0;}
		glColor4f(0.4,0.4,0.4,1);
		uiRectDraw(px,py,SLIDER_Z,sx,SLIDER_WIDTH);
		/*draw the slider handle */
		if(self->inner_sizex > 0){
			float handle_sx = sx * self->sizex / self->inner_sizex;
			float handle_px  = self->dx *self->sizex / self->inner_sizex;
			glColor4f(0.5,0.5,0.5,1);
			uiRectDraw(handle_px+SLIDER_MARGIN,py,SLIDER_Z+0.1,handle_sx,SLIDER_WIDTH);
		}
	}
	glDisable(GL_DEPTH_TEST);
}

static int uiRegionMotion(uiEntity *self,float x, float y, float p){
	float dx,dy;
	if(uiStateMouse(UI_MOUSE_BUTTON_2)){
		uiStateMouseDelta(&dx,&dy,NULL);
		self->dx += dx;
		self->dy += dy;
		return 0;
	}
	return 1;
}
uiEntity *uiRegionNew(const char *name,float inner_sizex, float inner_sizey){
	uiEntity *r = uiEntityNew(name,UI_ENT_REGION);
	r->inner_sizex = inner_sizex;
	r->inner_sizey = inner_sizey;
	uiEntitySetSize(r,100,100);
	r->draw = uiRegionDraw;
	r->motion = uiRegionMotion;
	return r;
}	
static void uiRectEntDraw(uiEntity *self){
	glColor4f(self->color[0],self->color[1],self->color[2],self->color[3]);
	uiRectDraw(0,0,0,self->sizex,self->sizey);
}
uiEntity *uiRectNew(const char *name, float sx, float sy, float red, float g, float b){
	uiEntity *r = uiEntityNew(name,UI_ENT_RECT);
	r->sizex = sx;
	r->sizey = sy;
	r->color[0] = red;
	r->color[1] = g;
	r->color[2] = b;
	r->color[3] = 1;
	r->draw = uiRectEntDraw;
	return r;
}

uiEntity *uiDivNew(const char *name, float sx, float sy){
	uiEntity *d = uiEntityNew(name,UI_ENT_DIV);
	d->sizex = sx;
	d->sizey = sy;
	return d;
}

static void uiTextEntryDraw(uiEntity *self){
	float x = 0;
	float y = 0;
	float z = 0;
	glColor4f(0,0,0,0.2);
	uiRectDraw(x+3,y-3,z-0.1,self->sizex,self->sizey);
	glColor4fv(uiWindowGetColor(UI_ITEM_COLOR,UI_NORMAL_COLOR));
	uiRectDraw(x,y,z,self->sizex,self->sizey);
	if(self->mouseover){
		glColor4f(1,0.2,0,1);
	}else{
		glColor4f(0,0,0,0.2);	
	}
	uiRectDraw(x+3,y+3,z,self->sizex-6,self->sizey-6);
	glColor4f(0,0,0,1);	
	uiStringDraw(self->string,0,0,0.1,self->sizex,self->sizey);	
}
uiEntity *uiTextEntryNew(const char *name, const char *text){
	uiEntity *l = uiEntityNew(name,UI_ENT_TEXTENTRY);
	l->string = uiStringNew(text,10,UI_BUTTON_WIDTH,UI_BUTTON_HEIGHT,5,11);
	l->draw = uiTextEntryDraw;
	uiEntitySetSize(l,UI_BUTTON_WIDTH,UI_BUTTON_HEIGHT);
	return l;
}
