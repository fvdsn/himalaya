#include <stdlib.h>
#include <stdio.h>
#include "gui/uiGui.h"
#include "uiHl.h"
extern float softness;
extern float radius;
extern float step;
extern float alpha;
extern float randomness;
extern float color[4];

extern float hlpx; 
extern float hlpy;
extern float hlsx;
extern float hlsy;
extern int   hlzoomlevel;
extern int   hlopcount;

uiEntity *canvas = NULL;

static uiEntity *uiColorPanel(void){
	uiEntity *p;
	uiEntity *e;
	p = uiPanelNew("ColorPanel");
	uiEntityAlign(p,UI_ALIGN_NORTH);
	uiEntityFitX(p,1);
	p->margin_out = 10;
		
	e = uiButtonNew("test",42,NULL);
	uiEntitySetPos(e,80,10);
	uiEntityAdd(e,p);
	
	e = uiButtonNew("yo",6666,NULL);
	uiEntitySetPos(e,10,10);
	uiEntityAdd(e,p);

	e = uiSliderNew("A",3,0.005,0.99,0.025,0,color+3,NULL);
	uiEntitySetPos(e,10,34);
	uiEntityAdd(e,p);
	
	e = uiSliderNew("B",2,0.005,0.99,0.025,0,color+2,NULL);
	uiEntitySetPos(e,10,54);
	uiEntityAdd(e,p);

	e = uiSliderNew("G",1,0.005,0.99,0.025,0,color+1,NULL);
	uiEntitySetPos(e,10,74);
	uiEntityAdd(e,p);

	e = uiSliderNew("R",2,0.005,0.99,0.025,0,color,NULL);
	uiEntitySetPos(e,10,94);
	uiEntityAdd(e,p);

	e = uiColorNew("colorpicker",color);
	uiEntitySetPos(e,80,50);
	uiEntitySetSize(e,60,60);
	uiEntityAdd(e,p);

	return p;
}
static uiEntity *uiStatPanel(void){
	uiEntity *p;
	uiEntity *e;
	p = uiPanelNew("StatPanel");
	uiEntitySetSize(p,UI_PANEL_WIDTH,UI_PANEL_HEIGHT+16);
	uiEntityAlign(p,UI_ALIGN_NORTH);
	uiEntityFitX(p,1);
	p->margin_out = 10;
		
	e = uiDisplayFloatNew("Px",&hlpx);
	uiEntityFitX(e,1);
	uiEntityAlign(e,UI_ALIGN_NORTH);
	e->margin_out = 5;
	uiEntityAdd(e,p);

	e = uiDisplayFloatNew("Py",&hlpy);
	uiEntityFitX(e,1);
	uiEntityAlign(e,UI_ALIGN_NORTH);
	e->margin_out = 5;
	uiEntityAdd(e,p);
	
	e = uiDisplayFloatNew("Sx",&hlsx);
	uiEntityFitX(e,1);
	uiEntityAlign(e,UI_ALIGN_NORTH);
	e->margin_out = 5;
	uiEntityAdd(e,p);

	e = uiDisplayFloatNew("Sy",&hlsy);
	uiEntityFitX(e,1);
	uiEntityAlign(e,UI_ALIGN_NORTH);
	e->margin_out = 5;
	uiEntityAdd(e,p);

	e = uiDisplayIntNew("Zoom Level",&hlzoomlevel);
	uiEntityFitX(e,1);
	uiEntityAlign(e,UI_ALIGN_NORTH);
	e->margin_out = 5;
	uiEntityAdd(e,p);

	e = uiDisplayIntNew("Op. Count",&hlopcount);
	uiEntityFitX(e,1);
	uiEntityAlign(e,UI_ALIGN_NORTH);
	e->margin_out = 5;
	uiEntityAdd(e,p);

	return p;
}
static uiEntity *uiToolPanel(void){
	uiEntity *p;
	uiEntity *s;
	p = uiPanelNew("ToolPanel");
	uiEntityAlign(p,UI_ALIGN_NORTH);
	uiEntityFitX(p,1);
	p->margin_out = 10;
		

	s = uiSliderNew("Softness",1,0.0,0.99,0.01,0,&softness,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	uiEntityFitX(s,1);
	s->margin_out = 5;
	uiEntityAdd(s,p);

	s = uiSliderNew("Radius",1,1,100000000,0.01,2,&radius,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	uiEntityFitX(s,1);
	s->margin_out = 5;
	uiEntityAdd(s,p);

	s = uiSliderNew("Step",1,0.1,2,0.025,0,&step,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	uiEntityFitX(s,1);
	s->margin_out = 5;
	uiEntityAdd(s,p);

	s = uiSliderNew("Randomness",1,0,5,0.025,0,&randomness,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	uiEntityFitX(s,1);
	s->margin_out = 5;
	uiEntityAdd(s,p);

	s = uiSliderNew("Opacity",1,0.01,1,0.025,0,&alpha,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	uiEntityFitX(s,1);
	s->margin_out = 5;
	uiEntityAdd(s,p);

	return p;
}
static uiEntity *uiSideBar(void){
	uiEntity *te;
	uiEntity *t;

	te = uiTabEnvNew("sidebar");
	uiEntitySetSize(te,180,1);
	uiEntityFitY(te,1);
	uiEntityAlign(te,UI_ALIGN_EAST);
	te->margin_out = 2;
	
	t = uiDivNew("Tool",180,1);
	uiEntityFitX(t,1);
	uiEntityFitY(t,1);
	uiTabAdd(t,te);

	uiEntityAdd(uiColorPanel(),t);
	uiEntityAdd(uiToolPanel(),t);
	uiEntityAdd(uiStatPanel(),t);

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
#define SIZEX 800000000.0
#define SIZEY 600000000.0
	hlCS cs = hlNewCS(HL_8B,HL_RGB);
	hlColor white = hlNewColor(cs,1,1,1,0,1);
	hlColor gray = hlNewColor(cs,0.5,0.5,0.5,0,1);
	hlImg *img	= hlNewImg(gray,SIZEX,SIZEY);
	hlOp *op = NULL;
	uiEntity *r  = uiDivNew("MainScreen",500,500);
	uiEntity *hl = uiHlNew("test",img,0,800,600);
	canvas = hl;
	op = hlNewOp(HL_DRAW_RECT);
		hlOpSetAllValue(op,"pos_tl",0.0,0.0);
		hlOpSetAllValue(op,"pos_br",SIZEX,SIZEY);
		hlOpSetAllValue(op,"alpha",1.0);
		hlOpSetAllColor(op,"fill_color",white);
	hlImgPushOp(img,op);
	uiHlBaseState(hl);

	uiEntitySetPos(hl,10,10);
	uiEntityAdd(hl,r);
	uiEntityFitX(r,1);
	uiEntityFitY(r,1);
	uiEntityFitX(hl,1);
	uiEntityFitY(hl,1);
	hl->margin_out = 5;
	r->margin_out = 2;
	return r;
}

int main(int argc, char **argv){
	uiEntity *s;
	hlInit();
	uiNewWindow("Himalaya Alpha 1",800,600);
	s = uiScreenNew("MainScreen");
	uiScreenSet(s);	
	uiEntityAdd(uiSideBar(),s);
	uiEntityAdd(uiMainScreen(),s);
	uiHlLog("logfile.log");
	if(argc == 2){
		fprintf(stdout,"REPLAY: logfile: %s \n",argv[1]);
		uiHlReplayLog(argv[1],canvas);
		fprintf(stdout,"REPLAY: end\n");
	}
	uiMainLoop();
	return 0;
}
	
