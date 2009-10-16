#include <stdlib.h>
#include <stdio.h>
#include "gui/uiGui.h"
#include "uiHl.h"
extern float softness;
extern float radius;
extern float step;
extern float alpha;
extern float color[4];

static uiEntity *uiColorPanel(void){
	uiEntity *p;
	uiEntity *e;
	p = uiPanelNew("panel");
		
	e = uiButtonNew("test",42,NULL);
	uiEntitySetPos(e,80,10);
	uiEntityAdd(e,p);
	
	e = uiButtonNew("yo",6666,NULL);
	uiEntitySetPos(e,10,10);
	uiEntityAdd(e,p);

	e = uiSliderNew("Alpha",3,0.005,0.99,0.025,color+3,NULL);
	uiEntitySetPos(e,10,34);
	uiEntityAdd(e,p);
	
	e = uiSliderNew("Blue",2,0.005,0.99,0.025,color+2,NULL);
	uiEntitySetPos(e,10,54);
	uiEntityAdd(e,p);

	e = uiSliderNew("Green",1,0.005,0.99,0.025,color+1,NULL);
	uiEntitySetPos(e,10,74);
	uiEntityAdd(e,p);

	e = uiSliderNew("Red",2,0.005,0.99,0.025,color,NULL);
	uiEntitySetPos(e,10,94);
	uiEntityAdd(e,p);

	e = uiColorNew("colorpicker",color);
	uiEntitySetPos(e,80,50);
	uiEntitySetSize(e,60,60);
	uiEntityAdd(e,p);

	return p;
}
static uiEntity *uiSideBar(void){
	uiEntity *te;
	uiEntity *t;
	uiEntity *s;

	te = uiTabEnvNew("sidebar");
	uiEntitySetSize(te,180,1);
	uiEntityFitY(te,1);
	uiEntityAlign(te,UI_ALIGN_EAST);
	te->margin_out = 2;
	
	t = uiRegionNew("Tool",180,1);
	uiEntityFitX(t,1);
	uiEntityFitY(t,1);
	uiTabAdd(t,te);

	s = uiSliderNew("Softness",1,0.01,0.99,0.025,&softness,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	s->margin_out = 4;
	uiEntityAdd(s,t);

	s = uiSliderNew("Radius",1,1,250,1,&radius,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	s->margin_out = 4;
	uiEntityAdd(s,t);

	s = uiSliderNew("Step",1,0.1,2,0.025,&step,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	s->margin_out = 4;
	uiEntityAdd(s,t);

	s = uiSliderNew("Opacity",1,0.01,1,0.025,&alpha,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	s->margin_out = 4;
	uiEntityAdd(s,t);

	uiEntityAdd(uiColorPanel(),t);

	t = uiRegionNew("Layers",180,1);
	uiEntityFitX(t,1);
	uiEntityFitY(t,1);
	uiTabAdd(t,te);

	t = uiRegionNew("History",180,1);
	uiEntityFitX(t,1);
	uiEntityFitY(t,1);
	uiTabAdd(t,te);

	return te;
}
static uiEntity *uiMainScreen(void){
	hlCS cs = hlNewCS(HL_8B,HL_RGB);
	hlColor c = hlNewColor(cs,0.5,1,0,0,0.1);
	hlColor white = hlNewColor(cs,1,1,1,1,1);
	hlRaw *buffer 	= hlNewRaw(cs,600,500);
	hlImg *img	= hlNewImg(white,100000,100000);
	hlRawFill(buffer,&c);
	uiEntity *r  = uiRegionNew("MainScreen",500,500);
	uiEntity *hl = uiHlNew("test",img,0,buffer);
	uiEntitySetPos(hl,100,100);
	uiEntityAdd(hl,r);
	uiEntityFitX(r,1);
	uiEntityFitY(r,1);
	r->margin_out = 2;
	return r;
}

int main(int argc, char **argv){
	uiEntity *s;
	uiNewWindow("Himalaya Alpha 1",800,600);
	s = uiScreenNew("MainScreen");
	uiScreenSet(s);	
	uiEntityAdd(uiSideBar(),s);
	uiEntityAdd(uiMainScreen(),s);
	hlInit();
	uiMainLoop();
	return 0;
}
	
