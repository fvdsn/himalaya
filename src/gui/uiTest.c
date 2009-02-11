#include <stdlib.h>
#include <stdio.h>
#include "uiCore.h"
#include "uiWidget.h"

float color[4] = {1.0,0.0,0.0,0.5};
void button_click(uiEntity*self, int id){
	printf("CLICK! : %d\n",id);
}
int main(int argc, char **argv){
	uiEntity *p;
	uiEntity *e;
	uiNewWindow("HelloWorld",600,400);
	
	p = uiPanelNew("panel",NULL);
	uiEntitySetPos(p,200,50);
	uiEntityAdd(p);

	e = uiButtonNew("test",p,42,button_click);
	uiEntitySetPos(e,80,10);
	uiEntityAdd(e);

	e = uiButtonNew("yo",p,6666,button_click);
	uiEntitySetPos(e,10,10);
	uiEntityAdd(e);

	e = uiSliderNew("Alpha",p,3,0,1,0.025,color+3,NULL);
	uiEntitySetPos(e,10,34);
	uiEntityAdd(e);

	e = uiSliderNew("Blue",p,2,0,1,0.025,color+2,NULL);
	uiEntitySetPos(e,10,54);
	uiEntityAdd(e);

	e = uiSliderNew("Green",p,1,0,1,0.025,color+1,NULL);
	uiEntitySetPos(e,10,74);
	uiEntityAdd(e);

	e = uiSliderNew("Red",p,2,0,1,0.025,color,NULL);
	uiEntitySetPos(e,10,94);
	uiEntityAdd(e);

	e = uiColorNew("colorpicker",p,color);
	uiEntitySetPos(e,80,50);
	uiEntitySetSize(e,60,60);
	uiEntityAdd(e);

	uiMainLoop();
	return 0;
}
