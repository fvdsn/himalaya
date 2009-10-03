#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cairo.h>
#include "uiCore.h"
#include "uiWidget.h"

void uiRectDraw(float x, float y, float z,float sx, float sy){
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,sy);
		glVertex3f(x,		y,		z);
		glTexCoord2f(sx,sy);
		glVertex3f(x+sx,	y,		z);
		glTexCoord2f(sx,0.0);
		glVertex3f(x+sx,	y+sy,		z);
		glTexCoord2f(0.0,0.0);
		glVertex3f(x,		y+sy,		z);
	glEnd();
}
static void uiTextureDraw(uiEntity *self){
	float x = self->tex_posx;
	float y = self->tex_posy;
	float z = 0.1;
	float sx = self->tex_sizex;
	float sy = self->tex_sizey;
	if(!self->tex){
		return;
	}
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glTexImage2D(	GL_TEXTURE_RECTANGLE_ARB,
			0,
			GL_RGBA,
			self->tex_sizex,
			self->tex_sizey,
			0,
			GL_BGRA,
			GL_UNSIGNED_BYTE,
			self->tex );
	uiRectDraw(x,y,z,sx,sy);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
}
static void uiButtonDraw(uiEntity *self){
	cairo_surface_t *surface;
	cairo_t *cr;
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
	if(!self->tex){
		printf("rendering texture\n");
		self->tex = malloc(self->tex_sizex*self->tex_sizey*4);
		memset(self->tex,0,self->tex_sizex*self->tex_sizey*4);
		surface = cairo_image_surface_create_for_data((unsigned char*)self->tex,
				CAIRO_FORMAT_ARGB32,
				self->tex_sizex,
				self->tex_sizey,
				4*self->tex_sizex);
		cr = cairo_create(surface);
		cairo_select_font_face(cr,"sans",CAIRO_FONT_SLANT_NORMAL,
						CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr,10.0);
		cairo_set_source_rgb(cr,0,0,0);
		cairo_move_to(cr,5,11);
		cairo_show_text(cr,self->name);
		cairo_destroy(cr);
		cairo_surface_finish(surface);
	}
	uiTextureDraw(self);
}
static int uiButtonClick(uiEntity *self,int button, int down, float x, float y, float pressure){
	uiButtonData *bd = (uiButtonData*)self->data;
	printf("click!:%s\tbutton:%d\tdown:%d\n",self->name,button,down);
	if(bd->click && button == 0 && down == UI_KEY_UP){
		bd->click(self,bd->id);
	}
	return 0;
}
uiEntity *uiScreenNew(char *name){
	uiEntity *s = uiEntityNew(name,UI_ENT_SCREEN);
	uiEntitySetSize(s,10000,10000);
	uiEntitySetPos(s,0,0);
	return s;
}
uiEntity *uiButtonNew(char *name,
		int id,
		void(*click)(uiEntity *self, int id)){
	uiButtonData *bd = (uiButtonData*)malloc(sizeof(uiButtonData));
	uiEntity *b = uiEntityNew(name,UI_ENT_BUTTON);
	uiEntitySetSize(b,60,16);
	bd->id = id;
	bd->click = click;
	b->data = bd;
	b->draw = uiButtonDraw;
	b->click = uiButtonClick;
	b->tex_posx = 0;
	b->tex_posy = 0;
	b->tex_sizex = b->sizex;
	b->tex_sizey = b->sizey;
	return b;
}
static void uiLabelDraw(uiEntity *self){
	uiLabelData *d = (uiLabelData*)self->data;
	float x = 0;
	float y = 0;
	float z = 0;
	/*uiTextDraw(x,y,z+0.1,
			d->font_size,
			uiWindowGetFont(d->font_police,d->font_style),
			d->font_color,
			d->text);*/
}
uiEntity *uiLabelNew(char *name, const char *text, const float *color,
		int font_police, int font_style, float font_size){
	uiEntity *l = uiEntityNew(name,UI_ENT_LABEL);
	uiLabelData *d = (uiLabelData*)malloc(sizeof(uiLabelData));
	memset(d,0,sizeof(uiLabelData));
	uiEntitySetSize(l,128,30);
	memcpy(d->font_color,color,4*sizeof(float));
	strncpy(d->text,text,UI_TEXT_LENGTH);
	d->font_police = font_police;
	d->font_style  = font_style;
	d->font_size   = font_size;
	l->data = d;
	l->draw = uiLabelDraw;
	return l;
}
uiEntity *uiCairoLabelNew(char *name, const char *text, const float *color,
		float font_size){
	uiEntity *l = uiEntityNew(name,UI_ENT_CAIROLABEL);
	uiLabelData *d = (uiLabelData*)malloc(sizeof(uiLabelData));
	memset(d,0,sizeof(uiLabelData));
	uiEntitySetSize(l,128,30);
	memcpy(d->font_color,color,4*sizeof(float));
	strncpy(d->text,text,UI_TEXT_LENGTH);
	/*TODO
	d->font_police = font_police;
	d->font_style  = font_style;
	*/
	d->font_size   = font_size;
	l->data = d;
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
uiEntity *uiPanelNew(char *name){
	uiEntity *p = uiEntityNew(name,UI_ENT_PANEL);
	uiEntitySetSize(p,150,120);
	p->draw = uiPanelDraw;
	p->motion = uiPanelMotion;
	return p;
}
static int uiSliderMotion(uiEntity *self,float x, float y, float p){
	float dx,v;
	uiSliderData *sd = (uiSliderData*)self->data;
	if(uiStateMouse(UI_MOUSE_BUTTON_1)){
		uiStateMouseDelta(&dx,NULL,NULL);
		v = sd->value + dx*sd->update_speed;
		if(v > sd->max_value){
			sd->value = sd->max_value;
		}else if (v < sd->min_value){
			sd->value = sd->min_value;
		}else{
			sd->value = v;
		}
		if(dx != 0.0){
			if (sd->slide){
				sd->slide(self,v,sd->id);
			}else if(sd->dest_value){
				*(sd->dest_value) = v;
			}
		}
	}
	return 0;
}
static void uiSliderDraw(uiEntity *self){
	cairo_surface_t *surface;
	cairo_t *cr;
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
	uiRectDraw(	x + margin, y + margin,z,
			(self->sizex - 2*margin)
				*(sd->value/(sd->max_value-sd->min_value)) ,
			self->sizey - 2*margin	);

	if(!self->tex){
		printf("rendering texture\n");
		self->tex = malloc(self->tex_sizex*self->tex_sizey*4);
		memset(self->tex,0,self->tex_sizex*self->tex_sizey*4);
		surface = cairo_image_surface_create_for_data((unsigned char*)self->tex,
				CAIRO_FORMAT_ARGB32,
				self->tex_sizex,
				self->tex_sizey,
				4*self->tex_sizex);
		cr = cairo_create(surface);
		cairo_select_font_face(cr,"sans",CAIRO_FONT_SLANT_NORMAL,
						CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr,10.0);
		cairo_set_source_rgba(cr,0.0,0.0,0.0,1.0);
		cairo_move_to(cr,5,11);
		cairo_show_text(cr,self->name);
		cairo_destroy(cr);
		cairo_surface_finish(surface);
	}
	uiTextureDraw(self);
}

uiEntity *uiSliderNew(char *name,
		int id,
		float min_value,
		float max_value,
		float update_speed,
		float *dest_value,
		void(*slide)(uiEntity *self, float value, int id)){
	uiSliderData *sd = (uiSliderData*)malloc(sizeof(uiSliderData));
	uiEntity *b = uiEntityNew(name,UI_ENT_SLIDER);
	uiEntitySetSize(b,60,16);
	sd->id = id;
	sd->slide = slide;
	sd->min_value = min_value;
	sd->max_value = max_value;
	sd->update_speed = update_speed;
	sd->dest_value = dest_value;
	if(dest_value){
		sd->value = *dest_value;
	}else{
		sd->value = ( max_value + min_value ) / 2.0;
	}
	b->data = sd;
	b->draw = uiSliderDraw;
	b->motion = uiSliderMotion;
	b->tex_posx = 0;
	b->tex_posy = 0;
	b->tex_sizex = b->sizex;
	b->tex_sizey = b->sizey;
	return b;
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

uiEntity *uiColorNew(char *name, float *color){
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
	uiRegionData *rd = (uiRegionData*)self->data;
	glColor4f(0.45,0.45,0.45,1);
	uiRectDraw(0,0,0,self->sizex,self->sizey);
	/*vertical slider */
	glEnable(GL_DEPTH_TEST);
	if(self->dy < 0 || self->dy + rd->inner_sizey > self->sizey){
		/*we need to draw a slider */
		/*draw the slider background */
		float px = self->sizex - SLIDER_WIDTH - SLIDER_MARGIN;
		float py = SLIDER_MARGIN;
		float sy = self->sizey - (2*SLIDER_MARGIN);
		if(sy <0){ sy = 0;}
		glColor4f(0.4,0.4,0.4,1);
		uiRectDraw(px,py,SLIDER_Z,SLIDER_WIDTH,sy);
		/*draw the slider handle */
		if(rd->inner_sizey > 0){
			float handle_sy = sy * self->sizey / rd->inner_sizey;
			float handle_py  = self->dy *self->sizey / rd->inner_sizey;
			glColor4f(0.5,0.5,0.5,1);
			uiRectDraw(px,handle_py+SLIDER_MARGIN,SLIDER_Z+0.1,SLIDER_WIDTH,handle_sy);
		}
	}
	/*horizontal slider */
	if(self->dx < 0 || self->dx + rd->inner_sizex > self->sizex){
		/*we need to draw a slider */
		/*draw the slider background */
		float py = SLIDER_MARGIN;
		float px = SLIDER_MARGIN;
		float sx = self->sizex - (2*SLIDER_MARGIN);
		if(sx <0){ sx = 0;}
		glColor4f(0.4,0.4,0.4,1);
		uiRectDraw(px,py,SLIDER_Z,sx,SLIDER_WIDTH);
		/*draw the slider handle */
		if(rd->inner_sizex > 0){
			float handle_sx = sx * self->sizex / rd->inner_sizex;
			float handle_px  = self->dx *self->sizex / rd->inner_sizex;
			glColor4f(0.5,0.5,0.5,1);
			uiRectDraw(handle_px+SLIDER_MARGIN,py,SLIDER_Z+0.1,handle_sx,SLIDER_WIDTH);
		}
	}
	glDisable(GL_DEPTH_TEST);
}

static int uiRegionMotion(uiEntity *self,float x, float y, float p){
	float dx,dy;
	uiRegionData *rd = (uiRegionData*)self->data;
	if(uiStateMouse(UI_MOUSE_BUTTON_1)){
		uiStateMouseDelta(&dx,&dy,NULL);
		if(self->sizex < rd->inner_sizex){
			if(self->dx + dx < 0){
				self->dx = 0;
			}else if(self->dx + dx + self->sizex > rd->inner_sizex){
				self->dx = rd->inner_sizex - self->sizex;
			}else{
				self->dx += dx;
			}
		}
		if(self->sizey < rd->inner_sizey){
			if(self->dy + dy < 0){
				self->dy = 0;
			}else if(self->dy + dy + self->sizey > rd->inner_sizey){
				self->dy = rd->inner_sizey - self->sizey;
			}else{
				self->dy += dy;
			}
		}
	}
	return 0;
}
uiEntity *uiRegionNew(char *name,float inner_sizex, float inner_sizey){
	uiEntity *r = uiEntityNew(name,UI_ENT_REGION);
	uiRegionData *rd = (uiRegionData*)malloc(sizeof(uiRegionData));
	rd->inner_sizex = inner_sizex;
	rd->inner_sizey = inner_sizey;
	uiEntitySetSize(r,100,100);
	r->draw = uiRegionDraw;
	r->data = rd;
	r->motion = uiRegionMotion;
	return r;
}	
static void uiRectEntDraw(uiEntity *self){
	glColor4f(self->color[0],self->color[1],self->color[2],self->color[3]);
	uiRectDraw(0,0,0,self->sizex,self->sizey);
}
uiEntity *uiRectNew(char *name, float sx, float sy, float red, float g, float b){
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

