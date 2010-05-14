#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "gui/uiGui.h"
#include "uiHl.h"
extern float softness;
extern float radius;
extern float step;
extern float alpha;
extern float randomness;
extern float color[4];
extern float hsva[4];

extern float hlpx; 
extern float hlpy;
extern float hlsx;
extern float hlsy;
extern int   hlzoomlevel;
extern int   hlopcount;
uiEntity *hslider = NULL;
uiEntity *sslider = NULL;
uiEntity *lslider = NULL;

uiEntity *radius_slider = NULL;
uiEntity *opacity_slider = NULL;
uiEntity *softness_slider = NULL;
uiEntity *randomness_slider = NULL;
uiEntity *step_slider = NULL;

uiEntity *canvas = NULL;
static void uiColorSlide(uiEntity *self, float value, int id){
	hsva[id] = value;
	hlHslToRgb(hsva,color);
	color[3] = hsva[3];
}
static uiEntity *uiColorPanel(void){
	uiEntity *p;
	uiEntity *e;
	p = uiPanelNew("ColorPanel");
	uiEntitySetSize(p,80,96);
	uiEntityAlign(p,UI_ALIGN_NORTH);
	uiEntityFitX(p,1);
	p->margin_out = 10;
		
	e = uiSliderNew("A",3,0.0,1.0,0.0025,0,1.0f,NULL,uiColorSlide);
	uiEntitySetPos(e,10,10);
	uiEntityAdd(e,p);
	
	e = uiSliderNew("Luminance",2,0.0,1.0,0.004,0,0.5f,NULL,uiColorSlide);
	uiEntitySetPos(e,10,30);
	uiEntityAdd(e,p);
	lslider = e;

	e = uiSliderNew("Saturation",1,0.0,1.0,0.004,0,0.5f,NULL,uiColorSlide);
	uiEntitySetPos(e,10,50);
	uiEntityAdd(e,p);
	sslider = e;

	e = uiSliderNew("Hue",0,0.0,1.0,0.004,0,0.0f,NULL,uiColorSlide);
	uiEntitySetPos(e,10,70);
	uiEntityAdd(e,p);
	hslider = e;

	e = uiColorNew("colorpicker",color);
	uiEntitySetPos(e,80,26);
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
		

	s = uiSliderNew("Softness",1,0.0,0.99,0.01,0,softness,&softness,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	uiEntityFitX(s,1);
	s->margin_out = 5;
	uiEntityAdd(s,p);
	softness_slider = s;

	s = uiSliderNew("Radius",1,1,100000000,0.01,2,radius,&radius,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	uiEntityFitX(s,1);
	s->margin_out = 5;
	uiEntityAdd(s,p);
	radius_slider = s;

	s = uiSliderNew("Step",1,0.1,2,0.025,0,step,&step,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	uiEntityFitX(s,1);
	s->margin_out = 5;
	uiEntityAdd(s,p);
	step_slider = s;

	s = uiSliderNew("Randomness",1,0,5,0.025,0,randomness,&randomness,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	uiEntityFitX(s,1);
	s->margin_out = 5;
	uiEntityAdd(s,p);
	randomness_slider = s;

	s = uiSliderNew("Opacity",1,0.01,1,0.025,0,alpha,&alpha,NULL);
	uiEntityAlign(s,UI_ALIGN_NORTH);
	uiEntityFitX(s,1);
	s->margin_out = 5;
	uiEntityAdd(s,p);
	opacity_slider = s;

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
	char replay_path[512];
	int  replay_file = 0;
	char log_path[512];
	int  log_file = 0;
	int  skip_renders = 0;
	int  no_gui = 0;
	int i = argc;
	hlInit();
	while(i--){
		if(!strncmp(argv[i],"--replay=",9)){
			if(!sscanf(argv[i],"--replay=%511s",replay_path)){
				fprintf(stderr,"ERROR: no path provided to --replay=PATH option\n");
				exit(1);
			}else{
				fprintf(stdout,"Replay file path: %s\n",replay_path);
				replay_file = 1;
			}
		}else if(!strncmp(argv[i],"--logfile=",10)){
			if(!sscanf(argv[i],"--logfile=%511s",log_path)){
				fprintf(stderr,"ERROR: no path provided to --logfile=PATH option\n");
				exit(1);
			}else{
				fprintf(stdout,"Log file path: %s\n",log_path);
				log_file = 1;
			}
		}else if(!strncmp(argv[i],"--boxcount=",11)){
			int box_count = 0;
			if(!sscanf(argv[i],"--boxcount=%d",&box_count)){
				fprintf(stderr,"ERROR: no integer provided to --boxcount=INT option\n");
				exit(1);
			}else{
				fprintf(stdout,"Box count : %d\n",box_count);
				uiHlSetBoxCount(box_count);
			}
		}else if(!strncmp(argv[i],"--box[",6)){
			int box_index = 0;
			int box_depth = 64;
			if(sscanf(argv[i],"--box[%d]=%d",&box_index,&box_depth)<2){
				fprintf(stderr,"ERROR: wrong format of --box[INT]=INT option\n");
				exit(1);
			}else{
				fprintf(stdout,"Box[%d] : %d\n",box_index,box_depth);
				uiHlSetBoxDepth(box_index,box_depth);
			}
		}else if(!strncmp(argv[i],"--skip-renders",14)){
			fprintf(stdout,"Skipping renders\n");
			skip_renders = 1;
		}else if(!strncmp(argv[i],"--no-gui",8)){
			no_gui = 1;
		}else if(!strncmp(argv[i],"--help",6)){
			fprintf(stdout,"OPTIONS :\n\t --skip-renders\n\t --logfile=PATH\n\t --replay=PATH\n\t --boxcount=INT\n\t --box[INT]=INT\n\t --no-gui\n\t --help\n");
			exit(0);
		}else if(i > 0){
			fprintf(stderr,"WARNING: unknown option '%s'\n try '--help' for more information\n",argv[i]);
		}
	}
	if(log_file){
		uiHlLog(log_path);
	}else{
		uiHlLog("/dev/null");
		fprintf(stdout,"Log file path: /dev/null\n");
	}
	uiNewWindow("Himalaya Alpha 1",800,600);
	s = uiScreenNew("MainScreen");
	uiScreenSet(s);	
	uiEntityAdd(uiSideBar(),s);
	uiEntityAdd(uiMainScreen(),s);
	if(replay_file){
		fprintf(stdout,"REPLAY: logfile: %s \n",argv[1]);
		uiHlReplayLog(replay_path,canvas,skip_renders);
		fprintf(stdout,"REPLAY: end\n");
	}
	if(!no_gui){
		uiMainLoop();
	}
	return 0;
}
	
