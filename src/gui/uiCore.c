#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include "uiCore.h"

SDL_Surface *window;
static int window_size_x;
static int window_size_y;
static int alive = 0;

static unsigned int uiTimeLeft(void){
	static unsigned int next_time = 0;
	unsigned int now = SDL_GetTicks();
	if(next_time <= now){
		next_time = now + UI_FRAME_INTERVAL;
		return 0;
	}else{
		return next_time - now;
	}
}

int  uiWindowGetSizeX(void){
	return window_size_x;
}
int  uiWindowGetSizeY(void){
	return window_size_y;
}

void uiNewWindow(const char *name, int sizex, int sizey){
	alive = 1;
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SwapBuffers();
	window = SDL_SetVideoMode(sizex,sizey,32,
			SDL_OPENGL|SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
	SDL_WM_SetCaption(name,NULL);
	window_size_x = sizex;
	window_size_y = sizey;
}
static void uiKeyDown(SDL_Event *event){
	char key = (char)(event->key.keysym.sym);
	uiStateSetKey(key,1);
}
static void uiKeyUp(SDL_Event *event){
	char key = (char)(event->key.keysym.sym);
	uiStateSetKey(key,0);
}
static void uiResize(int x, int y){
	window_size_x = x;
	window_size_y = y;
	SDL_FreeSurface(window);
	window = SDL_SetVideoMode(x,y,32,
			SDL_OPENGL|SDL_HWSURFACE|SDL_DOUBLEBUF|SDL_RESIZABLE);
}
void uiMainLoop(void){
	int button,x,y;
	while(alive){
		SDL_Event event;
		button = SDL_GetMouseState(&x,&y);
		y = uiWindowGetSizeY() - y - 1;
		uiStateSetMousePos(x,y,1.0);
		
		uiStateSetMouse(UI_MOUSE_BUTTON_1,button&SDL_BUTTON(1));
		uiStateSetMouse(UI_MOUSE_BUTTON_2,button&SDL_BUTTON(2));
		uiStateSetMouse(UI_MOUSE_BUTTON_3,button&SDL_BUTTON(3));
		

		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_KEYDOWN:
					break;
					uiKeyDown(&event);
				case SDL_KEYUP:
					uiKeyUp(&event);
					break;
				case SDL_VIDEORESIZE:
					uiResize(event.resize.w,event.resize.h);
					break;
				case SDL_QUIT:
					alive = 0;
					break;
				default:
					break;
			}
		}
		uiEventProcess();
		uiEntityCleanAll();
		uiWindowDrawBegin();
		uiEntityDrawAll();
		uiWindowDrawEnd();
		SDL_Delay(uiTimeLeft());
	}
}
