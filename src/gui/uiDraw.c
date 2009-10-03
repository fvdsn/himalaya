#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
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
