#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "uiCore.h"

float bg_color[4] = {0.5,0.5,0.5,1.0};
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
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
}
void uiWindowDrawEnd(void){
	SDL_GL_SwapBuffers();
}
