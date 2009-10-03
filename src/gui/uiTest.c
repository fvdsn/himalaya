#include <stdlib.h>
#include <stdio.h>
#include "uiCore.h"
#include "uiWidget.h"

float color[4] = {1.0,0.0,0.0,0.5};
float color2[4] = {1.0, 0.5, 0.0, 255.0 };
void button_click(uiEntity*self, int id){
	printf("CLICK! : %d\n",id);
}
int main(int argc, char **argv){
	uiEntity *p;
	uiEntity *r;
	uiEntity *l;
	uiEntity *e;
	uiEntity *s;
	uiNewWindow("HelloWorld",600,400);
	
	s = uiScreenNew("Main Screen");
	uiScreenSet(s);
	/*
	l = uiLabelNew("ABCDEF abcdef 0123456789 [éèû] <>?/;:.",NULL);
	uiEntitySetPos(l,100,50);
	uiEntityAdd(l);
	*/
	/*
	l = uiLabelNew("label",NULL,"ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 0123456789",color2,UI_FONT_SANS,UI_FONT_NORMAL,12.0);
	uiEntitySetPos(l,100,50);
	uiEntityAdd(l);
	*/
	r = uiRectNew("rect",25,100,1,0,1);
	uiEntitySetPos(r,250,50);
	uiEntityAlign(r,UI_ALIGN_NORTH);
	uiEntityFitX(r,1);
	r->margin_out = 5;
	uiEntityAdd(r,s);
	
	r = uiRegionNew("region",800,600);
	uiEntitySetPos(r,50,50);
	uiEntitySetSize(r,500,400);
	uiEntityFitX(r,1);
	uiEntityFitY(r,1);
	r->margin_out = 5;
	uiEntityAdd(r,s);

	p = uiPanelNew("panel");
	uiEntitySetPos(p,200,50);
	uiEntityAdd(p,r);
		
	e = uiButtonNew("test",42,button_click);
	uiEntitySetPos(e,80,10);
	uiEntityAdd(e,p);
	
	e = uiButtonNew("yo",6666,button_click);
	uiEntitySetPos(e,10,10);
	uiEntityAdd(e,p);

	e = uiSliderNew("Alpha",3,0,1,0.025,color+3,NULL);
	uiEntitySetPos(e,10,34);
	uiEntityAdd(e,p);
	
	e = uiSliderNew("Blue",2,0,1,0.025,color+2,NULL);
	uiEntitySetPos(e,10,54);
	uiEntityAdd(e,p);

	e = uiSliderNew("Green",1,0,1,0.025,color+1,NULL);
	uiEntitySetPos(e,10,74);
	uiEntityAdd(e,p);

	e = uiSliderNew("Red",2,0,1,0.025,color,NULL);
	uiEntitySetPos(e,10,94);
	uiEntityAdd(e,p);
	
	
	e = uiColorNew("colorpicker",color);
	uiEntitySetPos(e,80,50);
	uiEntitySetSize(e,60,60);
	uiEntityAdd(e,p);

	uiMainLoop();
	return 0;
}
