#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include "uiHl.h"
#include "gui/uiWidget.h"
#include <GL/gl.h>
#include <GL/glu.h>

#define BUTTON_PAINT  0
#define BUTTON_MOVE  1
#define BUTTON_PAN  2

extern uiEntity *hslider;
extern uiEntity *sslider;
extern uiEntity *lslider;

extern uiEntity *radius_slider;
extern uiEntity *opacity_slider;
extern uiEntity *softness_slider;
extern uiEntity *step_slider;
extern uiEntity *randomness_slider;

/*tool*/
float step = 0.2;
float radius = 32.0f;
float alpha = 1.0f;
float softness = 0.9f;
float randomness = 0.0f;
float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
float hsva[4]  = {0.5,0.5,0.5,0.5};

/*actions*/
enum actions{
	ACTION_OPACITY,
	ACTION_RADIUS,
	ACTION_SOFTNESS,
	ACTION_STEP,
	ACTION_RANDOMNESS,
	ACTION_PANNING,
	ACTION_HUE,
	ACTION_LUMINANCE,
	ACTION_SATURATION,
	ACTION_NONE
};
int   action_current = ACTION_NONE;
float action_px = 0.0f;
float action_py = 0.0f;
float action_value = 0.0f;

/*viewport size in hl coordinates*/
float hlpx = 0.0;
float hlpy = 0.0;
float hlsx = 0.0;
float hlsy = 0.0;
int   hlzoomlevel = 0;
int   hlopcount = 0;
FILE *logfile = NULL;
double starttime = 0.0;

/*box status*/
#define MAX_BOX_COUNT 10
int   box_count = 3;
int   box_depth[MAX_BOX_COUNT] = {16,64,512,INT_MAX,INT_MAX, INT_MAX,INT_MAX,INT_MAX,INT_MAX,INT_MAX};

void uiHlSetBoxCount(int bcount){
	if (bcount >= 0 && bcount <= MAX_BOX_COUNT){
		box_count = bcount;
	}else{
		fprintf(stderr,"WARNING: uiHlSetBoxCount(): box count %d out of range [0,%d]\n",bcount,MAX_BOX_COUNT);
	}
}
void uiHlSetBoxDepth(int index, int depth){
	if (index >= 0 && index < MAX_BOX_COUNT){
		if(depth > 0){
			box_depth[index] = depth;
		}else{
			box_depth[index] = INT_MAX;
		}
	}else{
		fprintf(stderr,"WARNING: uiHlSetBoxDepth(): index %d out of range [0,%d[\n",index,MAX_BOX_COUNT);
	}
}
void uiHlLog(const char *logfilepath){
	struct timeval time;
	logfile = fopen(logfilepath,"w");
	gettimeofday(&time,NULL);
	starttime = time.tv_sec + (time.tv_usec/1000000.0);
	if(!logfile){
		fprintf(stderr,"FAILURE: Could not open log file :\'%s\'\n",logfilepath);
	}
}
static double uiHlLogTime(void){
	struct timeval time;
	double ctime = 0.0;
	gettimeofday(&time,NULL);
	ctime = time.tv_sec + (time.tv_usec/1000000.0) - starttime;
	if(logfile){
		fprintf(logfile,"time %f\n",(float)(ctime));
	}
	return ctime;
}
static double uiHlLogRenderTime(double start){
	struct timeval time;
	double ctime = 0.0;
	double delta = 0.0;
	gettimeofday(&time,NULL);
	ctime = time.tv_sec + (time.tv_usec/1000000.0) - starttime;
	delta = ctime - start;
	if(logfile){
		fprintf(logfile,"rendertime %f\n",(float)(delta));
	}
	return delta;
}
#define UI_LOG(message) if(logfile){fprintf(logfile,message);uiHlLogTime();}
static void	uiHlRender(uiEntity *hl,int dx, int dy, int sx, int sy, int zoomlevel ){
	uiHlData *hd = (uiHlData*)hl->data;
	double start = 0.0;
	if(logfile){
		start = uiHlLogTime();
	}
	if(!hd->raw){
		hd->raw = hlNewRaw(hd->cs,sx,sy);
	}else if(hlRawSizeX(hd->raw)!= sx || hlRawSizeY(hd->raw)!= sy){
		hlFreeRaw(hd->raw);
		hd->raw = hlNewRaw(hd->cs,sx,sy);
	}
	hlImgRender(hd->img,0,hd->raw,dx,dy,zoomlevel);
	if(logfile){
		fprintf(logfile,"render %d %d %d %d %d\n",dx, dy, sx, sy, zoomlevel); 
		uiHlLogTime();
		uiHlLogRenderTime(start);
	}
}
static void uiHlPaintStart(uiEntity *self){
	uiHlData *hd = (uiHlData*)self->data;
	int i = box_count;
	hd->painting = 1;
	if(uiStateMod(UI_SHIFT)){
		hd->starting = 0;	/* continue where the previous stroke ended */
	}
	while(i--){
		hlImgPushOpenBBox(hd->img,box_depth[i]);
	}
	UI_LOG("paint_start\n")
}
static void uiHlPaintEnd(uiEntity *self){
	uiHlData *hd = (uiHlData*)self->data;
	uiHlPushState(self);
	hd->painting = 0;	
	hd->starting = 1;	/* next click will be beginning of new stroke */
	UI_LOG("paint_end\n")
}
static void uiHlPaintCircle(uiEntity *self, float x, float y, float radius_in, float radius_out, float opacity, float red, float green, float blue, float alpha){
	uiHlData *hd = (uiHlData*)self->data;
	hlColor col = hlNewColor( hd->cs,red,green,blue,0,alpha);
	hlOp* op = hlNewOp(HL_DRAW_CIRCLE);
	//printf("pos:%f,%f\n",x,y);
	hlOpSetAllValue(op,"pos_center",x,y);
	hlOpSetAllValue(op,"radius_in",radius_in);
	hlOpSetAllValue(op,"radius_out",radius_out);
	hlOpSetAllValue(op,"alpha",opacity);
	hlOpSetAllColor(op,"fill_color",col);
	hlImgPushOp(hd->img,op);
	hlopcount++;
	if(logfile){
		fprintf(logfile,"paint_circle %f %f %f %f %f %f %f %f %f\n",
			x,y,radius_in,radius_out,opacity,red,green,blue,alpha);
		uiHlLogTime();
	}
}
void uiHlReplayLog(const char *replayfilepath, uiEntity *hl,int skip_renders){
	FILE *log = fopen(replayfilepath,"r");
	//uiHlData *hd = (uiHlData*)hl->data;
	char line[1024];
	if(!log){
		fprintf(stderr,"FAILURE: Cannot read log file :\'%s\'\n",replayfilepath);
		return;
	}
	while(fgets(line,1024,log)){
		//fprintf(stdout,"REPLAY: %s",line);
		if(!strncmp(line,"paint_start\n",12)){
			uiHlPaintStart(hl);
			//fprintf(stdout,"COMMAND: paint_start\n");
		}else if(!strncmp(line,"paint_end\n",10)){
			uiHlPaintEnd(hl);
			//fprintf(stdout,"COMMAND: paint_end\n");
		}else if(!strncmp(line,"paint_circle ",13)){
			float x, y, radius_in, radius_out, opacity, red, green, blue, alpha;
			if( 9 == sscanf(line," paint_circle %f %f %f %f %f %f %f %f %f",
					&x,&y,&radius_in,&radius_out,&opacity,&red,&green,&blue,&alpha)){
				uiHlPaintCircle(hl,x,y,radius_in,radius_out,opacity,red,green,blue,alpha);
			//	fprintf(stdout,"COMMAND: paint_circle\n");
			}else{
				fprintf(stderr,"ERROR :  paint_circle has wrong number of arguments\n");
			}
		}else if(!strncmp(line,"render ",7) && !skip_renders){
			int dx,dy,sx,sy,zoomlevel;
			if( 5 == sscanf(line," render %d %d %d %d %d",&dx,&dy,&sx,&sy,&zoomlevel)){
				uiHlRender(hl,dx,dy,sx,sy,zoomlevel);
			//	fprintf(stdout,"COMMAND: render\n");
			}else{
				fprintf(stderr,"ERROR :  render has wrong number of arguments\n");
			}
		}else if(!strncmp(line,"undo\n",5)){
			uiHlUndo(hl);
			//fprintf(stdout,"COMMAND: undo\n");
		}else if(!strncmp(line,"redo\n",5)){
			uiHlRedo(hl);
			//fprintf(stdout,"COMMAND: redo\n");
		}else{
			//fprintf(stdout,"COMMAND: ignored\n");
		}
	}
}
static int uiHlMotion(uiEntity *self, float x, float y, float p){
	uiHlData *hd = (uiHlData*)self->data;
	float ralpha = 1.0f;
	uiEntityMousePos(self,&x,&y,NULL);
	/*for radius circle drawing */
	hd->bpx = x;
	hd->bpy = y;
	hd->br  = radius*powf(2,-hd->zoomlevel);
	if(action_current != ACTION_NONE){
		float dx = x-action_px;
		float fact = powf(1.0075,dx);
		float linfact  = dx > 0.0f ? powf(1.0075,dx) - 1.0f : -(powf(1.0075,-dx)-1.0f);
		if(action_current == ACTION_OPACITY){
			uiSliderSetValue(opacity_slider,action_value*fact);
		}else if(action_current == ACTION_RADIUS){
			uiSliderSetValue(radius_slider,action_value*fact);
		}else if(action_current == ACTION_SOFTNESS){
			uiSliderSetValue(softness_slider,action_value+linfact*0.5);
		}else if(action_current == ACTION_STEP){
			uiSliderSetValue(step_slider,action_value+linfact*0.5);
		}else if(action_current == ACTION_HUE){
			float v = fmodf(action_value+linfact*0.1,1.0f);
			v = v > 0.0f ? v : 1.0f + v;
			uiSliderSetValue(hslider,v);
		}else if(action_current == ACTION_SATURATION){
			uiSliderSetValue(sslider,action_value+linfact*0.5);
		}else if(action_current == ACTION_LUMINANCE){
			uiSliderSetValue(lslider,action_value+linfact*0.5);
		}else if(action_current == ACTION_PANNING){
			uiStateMouseDelta(&x,&y,NULL);
			hd->dx -= x;
			hd->dy += y;
			hd->uptodate = 0;
		}
		return UI_DONE;
	}
	if(hd->painting){
		/*position of click in hl zoom coordinates */
		float py =  (hd->sy - (y - hd->dy))*powf(2,hd->zoomlevel); 	
		float px =  (x + hd->dx)*powf(2,hd->zoomlevel);
		/*vector from last click in hl zoom coordinates */
		float dpx = (px-hd->lpx);	
		float dpy = (py-hd->lpy);
		float dplength = sqrtf(dpx*dpx + dpy*dpy);
		/* we prevent drawing if the stepsize is below 0.25 pixel in
		 * screenspace so that the user doesn't introduce billions of
		 * operations in one click */	
		if(step*radius*powf(2,-hd->zoomlevel) < 0.05){
			fprintf(stderr,"WARNING : step size is too small at this zoomlevel");
			return UI_DONE;
		}
		/*we determine ralpha from alpha*/
		if(alpha >= 1.0f){
			ralpha = 1.0f;
		}else{
			float layers = 1.0f/step;
			if(layers < 1.0f){
				layers = 1.0f;
			}
			ralpha = 1.0f - logf(1.0f-alpha)/logf(layers);
			//fprintf(stdout,"ralpha:%f\n",ralpha);
		}

		/* we draw the first brush disregarding the step between the
		 * last one */
		if(hd->starting){
			hd->lpx = px;
			hd->lpy = py;
			uiHlPaintCircle(self, px, py,radius*(1.0-softness),radius,alpha,color[0],color[1],color[2],color[3]);
			hd->uptodate = 0;
			hd->starting = 0;
			return UI_DONE;
		}
		/* we draw one brush every step between the two clicks */
		while(dplength > step*radius){
			float rpx = (randomness*radius)*(rand()/(float)RAND_MAX - 0.5f);
			float rpy = (randomness*radius)*(rand()/(float)RAND_MAX - 0.5f);
			float dstepx = dpx / dplength * step * radius;
			float dstepy = dpy / dplength * step * radius;
			hd->lpx += dstepx;
			hd->lpy += dstepy;
			dpx -= dstepx;
			dpy -= dstepy;
			dplength = sqrtf(dpx*dpx + dpy*dpy);
			uiHlPaintCircle(self, hd->lpx + rpx, hd->lpy +rpy,radius*(1.0-softness),radius,alpha,color[0],color[1],color[2],color[3]);
			hd->uptodate = 0;
		}
	}
	return UI_DONE;
}
static int uiHlKeyPress(uiEntity *self, int key, int down){
	uiHlData *hd = (uiHlData*)self->data;
	if(down == UI_KEY_UP){
		switch(key){
			case 'z':
				uiHlZoomUp(self);
				return 0;
			case 'x':
				uiHlZoomDown(self);
				return 0;
			case 'u':
				uiHlUndo(self);
				return 0;
			case 'r':
				uiHlRedo(self);
				return 0;
			case 'g':
				if(!hd->img){
					fprintf(stderr,"FAILURE: cannot print graph : image NULL \n");
				}else{
					FILE *f = NULL;
					printf("printing image graph ... ");
					f = fopen("img.dot","w");
					hlGraphImg(f,hd->img,HL_GRAPH_SIMPLE);
					fclose(f);
					printf("done \n");
				}
				return 0;
			case 'q':
				if(!hd->img){
					fprintf(stderr,"FAILURE: cannot colorpick: image NULL \n");
				}else{
					float x,y;
					int px = 0;
					int py = 0;
					hlColor col = hlNewColor(hlImgCS(hd->img),0,0,0,0,0);
					uiEntityMousePos(self,&x,&y,NULL);
					px = hd->dx + (int)x;
					//py = hd->dy + (int)(hd->sx - y);
					py = hd->sy - (y - hd->dy); 	
					fprintf(stdout,"[%d,%d]\n",px,py);
					hlImgColorPick(hd->img,0,px,py,hd->zoomlevel,&col);
					hlPrintColor(&col);
					color[0] = hlColorGetChan(&col,0);
					color[1] = hlColorGetChan(&col,1);
					color[2] = hlColorGetChan(&col,2);
					hlRgbToHsl(color,hsva);
					uiSliderSetValue(hslider,hsva[0]);
					uiSliderSetValue(sslider,hsva[1]);
					uiSliderSetValue(lslider,hsva[2]);

				}
				return 0;
			default:
				break;
		}
	}
	return 0;
}
static int uiHlClick(uiEntity *self, int button, int down, float x, float y, float p){
	if(button == 0 && down == UI_KEY_DOWN){
		if(uiStateKey('w') == UI_KEY_DOWN){
			uiEntityMousePos(self,&action_px,&action_py,NULL);
			if(uiStateMod(UI_SHIFT) == UI_KEY_DOWN){
				action_current = ACTION_OPACITY;
				action_value   = uiSliderGetValue(opacity_slider);
			}else if(uiStateMod(UI_CTRL) == UI_KEY_DOWN){
				action_current = ACTION_SOFTNESS;
				action_value   = uiSliderGetValue(softness_slider);
			}else if(uiStateMod(UI_ALT) == UI_KEY_DOWN){
				action_current = ACTION_STEP;
				action_value   = uiSliderGetValue(step_slider);
			}else{
				action_current = ACTION_RADIUS;
				action_value   = uiSliderGetValue(radius_slider);
			}
		}else if(uiStateKey('e') == UI_KEY_DOWN){
			uiEntityMousePos(self,&action_px,&action_py,NULL);
			if(uiStateMod(UI_SHIFT) == UI_KEY_DOWN){
				action_current = ACTION_SATURATION;
				action_value   = uiSliderGetValue(sslider);
			}else if(uiStateMod(UI_CTRL) == UI_KEY_DOWN){
				action_current = ACTION_HUE;
				action_value   = uiSliderGetValue(hslider);
			}else{
				action_current = ACTION_LUMINANCE;
				action_value   = uiSliderGetValue(lslider);
			}
		}else if(uiStateMod(UI_SPACE) == UI_KEY_DOWN){
			action_current = ACTION_PANNING;
		}else{
			uiHlPaintStart(self);
			uiHlMotion(self,x,y,p);
		}
	}else if(button == BUTTON_PAN && down == UI_KEY_DOWN){
		action_current = ACTION_PANNING;
	}else if(button == BUTTON_PAN && down == UI_KEY_UP){
		action_current = ACTION_NONE;
	}else if(button == 0 && down == UI_KEY_UP){
		if(action_current != ACTION_NONE){
			action_current = ACTION_NONE;
		}else{
			uiHlPaintEnd(self);
		}
	}
	return UI_DONE;
}
static void uiHlDraw(uiEntity *self){
	uiHlData*hd = (uiHlData*)self->data;
	if(!hd->img){
		return;
	}
	/*size of the entity has changed, resize the viewport*/
	if(!hd->raw || hd->sx != (int)self->sizex || hd->sy != (int)self->sizey){
		/*printf("viewport resize from [%d,%d] to [%d.%d]\n",hd->sx,hd->sy,(int)self->sizex,(int)self->sizey);*/
		hd->sx = (int)self->sizex;
		hd->sy = (int)self->sizey;
		hlFreeRaw(hd->raw);
		hd->raw = hlNewRaw(hd->cs,hd->sx,hd->sy);
		hd->uptodate = 0;
	}
	/* viewport needs to be redrawn */
	if(!hd->uptodate){
		uiHlRender(self,hd->dx,hd->dy,hd->sx,hd->sy,hd->zoomlevel);
		hd->uptodate=1;
		/*update viewport statistics*/
		hlpx = -hd->dx*powf(2,hd->zoomlevel);
		hlpy = hd->dy*powf(2,hd->zoomlevel);
		hlsx = hd->sx*powf(2,hd->zoomlevel);
		hlsy = hd->sy*powf(2,hd->zoomlevel);
	}
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glColor4f(1,1,1,1);
	glTexImage2D(	GL_TEXTURE_RECTANGLE_ARB,
			0,
			GL_RGBA,
			hd->sx,
			hd->sy,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			(char*)hlRawData(hd->raw) );
	uiRectDraw(0,0,0,hd->sx,hd->sy);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glEnable(GL_BLEND);
	if(self->mouseover){
		if(action_current == ACTION_NONE){
			glColor4f(0.0,0.0,0.0,0.1);
			glLineWidth(1.5f);
			uiDrawCircle(hd->bpx,hd->bpy,0.1,hd->br);
		}else if(action_current == ACTION_RADIUS){
			glColor4f(0.0,0.0,0.0,1);
			glLineWidth(2.0f);
			uiDrawCircle(hd->bpx,hd->bpy,0.1,hd->br);
		}else if(action_current == ACTION_SOFTNESS){
			glColor4f(0.0,0.0,0.0,0.75);
			glLineWidth(1.5f);
			uiDrawCircle(hd->bpx,hd->bpy,0.1,hd->br);
			glLineWidth(2.5f);
			uiDrawCircle(hd->bpx,hd->bpy,0.1,hd->br*(1.0f-softness));
		}else if(action_current == ACTION_OPACITY){
			glColor4f(0.0,0.0,0.0,0.2);
			uiRectDraw(hd->bpx-10,hd->bpy-52,0.1,20,104);
			glColor4f(0.0,0.0,0.0,0.5);
			uiRectDraw(hd->bpx-8,hd->bpy-50,0.1,16,100*alpha);
		}else if(action_current == ACTION_STEP){
			glColor4f(0.0,0.0,0.0,0.75);
			glLineWidth(2.5f);
			uiDrawCircle(hd->bpx,hd->bpy,0.1,hd->br);
			glLineWidth(1.5f);
			uiDrawCircle(hd->bpx+hd->br*step,hd->bpy,0.1,hd->br);
			uiDrawCircle(hd->bpx-hd->br*step,hd->bpy,0.1,hd->br);
		}else if(action_current == ACTION_HUE || action_current == ACTION_SATURATION || action_current == ACTION_LUMINANCE){
			glEnable(GL_POLYGON_SMOOTH);
			glColor4f(color[0],color[1],color[2],0.2);
			uiDrawDisc(action_px,action_py,0.1,32);
			glColor4f(color[0],color[1],color[2],1.0);
			uiDrawDisc(action_px,action_py,0.1,24);
			glDisable(GL_POLYGON_SMOOTH);
			uiDrawDisc(action_px,action_py,0.1,23);

		}
	}
}
uiEntity *uiHlNew(const char *name, hlImg *img, hlState s,int sx, int sy){
	uiEntity *h = uiEntityNew(name,UI_ENT_HL);
	uiHlData *hd = (uiHlData*)malloc(sizeof(uiHlData));
	memset((char*)hd,0,sizeof(uiHlData));
	h->data = hd;
	h->draw = uiHlDraw;
	h->click = uiHlClick;
	h->motion = uiHlMotion;
	h->key_press = uiHlKeyPress;
	hd->sx = sx;
	hd->sy = sy;
	hd->starting = 1;
	hd->hist = hlNewList();
	hd->hist_max_size = 8;
	uiEntitySetSize(h,sx,sy);
	uiHlSetImg(h,img,s);
	hd->raw = hlNewRaw(hd->cs,sx,sy);
	return h;
}
void	uiHlSetImg(uiEntity *hl, hlImg *img, hlState s){
	uiHlData *hd = (uiHlData*)hl->data;
	/*TODO assert correct entity type */
	hd->img = img;
	hd->state = s;
	hd->cs = hlImgCS(img);
	hd->zoomlevel = 0;
	hd->uptodate = 0;
}
void	uiHlSetState(uiEntity *hl, hlState s){
	/*TODO assert correct entity type */
	((uiHlData*)hl->data)->state = s;
	((uiHlData*)hl->data)->uptodate = 0;
}
void uiHlZoomUp(uiEntity *hl){
	uiHlData *hd = (uiHlData*)hl->data;
	float cx,cy;
	if(hd->zoomlevel < 31){
		hd->zoomlevel++;
		/* zooming on center of screen */	
		cx =  hd->dx + hd->sx/2;
		cy =  hd->dy + hd->sy/2;
		hd->dx = cx/2 - hd->sx/2;
		hd->dy = cy/2 - hd->sy/2;

		hlzoomlevel = hd->zoomlevel;
		hd->uptodate = 0;
		UI_LOG("zoomup\n")
		printf("zoomup\n");
	}
}
void uiHlZoomDown(uiEntity *hl){
	uiHlData *hd = (uiHlData*)hl->data;
	float cx,cy;
	if(hd->zoomlevel > 0){
		hd->zoomlevel--;

		/* zooming on center of screen */	
		cx = hd->dx + hd->sx/2;
		cy =  hd->dy + hd->sy/2;
		hd->dx = cx*2 - hd->sx/2;
		hd->dy = cy*2 - hd->sy/2;

		hlzoomlevel = hd->zoomlevel;
		hd->uptodate = 0;
		UI_LOG("zoomdown\n")
	}
}
void uiHlBaseState(uiEntity *hl){
	uiHlData *hd = (uiHlData*)hl->data;
	if(hd->hist_cur_index != 0 && hd->hist->size != 0){
		fprintf(stderr,"ERROR: Cannot set base state on non empty history\n");
	}else{
		hlListAppend(hd->hist,(void*)hlImgStateSave(hd->img));
		hd->hist_cur_index = 0;
	}
}
void uiHlPushState(uiEntity *hl){
	uiHlData *hd = (uiHlData*)hl->data;
	while(hd->hist_cur_index < hd->hist->size -1){
		// we remove all states above 
		hlState s = (hlState)hlListRem(hd->hist,hd->hist->size -1);
		hlImgStateRem(hd->img,s);
	}
	hlListAppend(hd->hist,(void*)hlImgStateSave(hd->img));
	
	if(hd->hist->size > hd->hist_max_size){
		hlState s = (hlState)hlListRem(hd->hist,1);
		hlImgStateRem(hd->img,s);
	}
	hd->hist_cur_index = hd->hist->size -1;
}
void uiHlUndo(uiEntity *hl){
	uiHlData *hd = (uiHlData*)hl->data;
	if(hd->hist_cur_index > 0){
		hd->hist_cur_index--;
		hlImgStateLoad(hd->img,(hlState)hlListGet(hd->hist,hd->hist_cur_index));
		hd->uptodate = 0;
		UI_LOG("undo\n")
	}
}
void uiHlRedo(uiEntity *hl){
	uiHlData *hd = (uiHlData*)hl->data;
	if(hd->hist_cur_index < hd->hist->size -1){
		hd->hist_cur_index++;
		hlImgStateLoad(hd->img,(hlState)hlListGet(hd->hist,hd->hist_cur_index));
		hd->uptodate = 0;
		UI_LOG("redo\n")
	}
}
	
