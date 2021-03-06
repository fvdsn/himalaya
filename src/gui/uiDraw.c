#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include "uiCore.h"

float bg_color[4] = {0.3,0.3,0.3,1.0};
float font_color[4] = {0.0,0.0,0.0,1.0};
float item_color[4] = {0.8,0.8,0.8,1.0};
float panel_color[4] = {0.4,0.4,0.4,1.0};

float *uiWindowGetColor(int type, int modifier){
	switch(type){
		case UI_BG_COLOR:
			return bg_color;
		case UI_FONT_COLOR:
			return font_color;
		case UI_ITEM_COLOR:
			return item_color;
		case UI_PANEL_COLOR:
			return panel_color;
		default:
			return NULL;
	}
}
/*
int font_loaded = 0;

uiFont * font[UI_FONT_FACE_COUNT * UI_FONT_TYPE_COUNT];
#define FONT(face,type) font[UI_FONT_TYPE_COUNT*(face) + (type)] 

uiFont *uiWindowGetFont(int face, int type){
	if(!font_loaded){
		FONT(UI_FONT_SANS,UI_FONT_NORMAL) = uiFontLoad("fonts/Bitstream Vera Sans", DEFAULT_FONT_SIZE);
		FONT(UI_FONT_SANS,UI_FONT_BOLD)   = uiFontLoad("fonts/Bitstream Vera Sans, Bold", DEFAULT_FONT_SIZE);
		FONT(UI_FONT_SANS,UI_FONT_ITALIC)   = uiFontLoad("fonts/Bitstream Vera Sans, Oblique", DEFAULT_FONT_SIZE);
		FONT(UI_FONT_SANS,UI_FONT_BOLD_ITALIC)   = uiFontLoad("fonts/Bitstream Vera Sans, Bold Oblique", DEFAULT_FONT_SIZE);

		FONT(UI_FONT_MONO,UI_FONT_NORMAL) = uiFontLoad("fonts/Inconsolata",DEFAULT_FONT_SIZE);
		FONT(UI_FONT_MONO,UI_FONT_BOLD) = FONT(UI_FONT_MONO,UI_FONT_NORMAL);
		FONT(UI_FONT_MONO,UI_FONT_ITALIC) = FONT(UI_FONT_MONO,UI_FONT_NORMAL);
		FONT(UI_FONT_MONO,UI_FONT_BOLD_ITALIC) = FONT(UI_FONT_MONO,UI_FONT_NORMAL);
		font_loaded = 1;
	}
	return FONT(face,type);
}*/
void uiWindowDrawBegin(void){
	glViewport(0,0,(GLsizei)uiWindowGetSizeX(),(GLsizei)uiWindowGetSizeY());
	glClearColor(bg_color[0],bg_color[1],bg_color[2],bg_color[3]);	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(	0.0,
			uiWindowGetSizeX(),
			0.0,
			uiWindowGetSizeY(),
			-100,100	);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}
void uiWindowDrawEnd(void){
	SDL_GL_SwapBuffers();
}
void uiDrawCircle(float x, float y, float z, float r){
	int segments = 32;
	float step = 2.0*3.141592/segments;
	float angle = 0.0f;
	glEnable(GL_LINE_SMOOTH);
	glBegin(GL_LINE_LOOP);
	while(segments--){
		glVertex3f(x+ cosf(angle)*r, y+sinf(angle)*r,z);
		angle += step;
	}
	glEnd();
	glDisable(GL_LINE_SMOOTH);

}
void uiDrawDisc(float x, float y, float z, float r){
	int segments = 32;
	float step = 2.0*3.141592/(segments-1);
	float angle = 0.0f;
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(x,y,z);
	while(segments--){
		glVertex3f(x+ cosf(angle)*r, y+sinf(angle)*r,z);
		angle += step;
	}
	glEnd();

}
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
