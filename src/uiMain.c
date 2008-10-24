#include <clutter/clutter.h>
#include <stdlib.h>
#include <math.h>
#include "hlImg.h"
#include "hlUndo.h"
#include "hlDebug.h"

int is_r_pressed = 0;
int is_g_pressed = 0;
int is_b_pressed = 0;
int is_a_pressed = 0;
int is_l_pressed = 0;
int is_s_pressed = 0;
int is_h_pressed = 0;
int is_mouse_pressed = 0;
int paint_radius = 20.0;
ClutterColor paint_color = { 255, 0, 0, 255};
ClutterActor *stage;
ClutterActor *rcanvas;
ClutterActor *color_picker;
float lastx = 0;
float lasty = 0;
float step  = 1;
int painting = 0;
hlRaw *canvas_data;
hlImg *canvas_img;
hlUndo *undo_stack;
hlCS cs; /*hlNewCS(HL_8B,HL_RGB);*/
hlColor clr;
hlColor paint_clr;
int zoomlevel;
int nextzoomlevel;

void canvas_repaint(void){
	if(nextzoomlevel ==zoomlevel){
	hlImgRenderToRaw(canvas_img,canvas_data,0,0,0,zoomlevel);
	clutter_texture_set_from_rgb_data(	CLUTTER_TEXTURE(rcanvas),
						(guchar*)hlRawData(canvas_data),
						TRUE,
						hlRawSizeX(canvas_data),hlRawSizeY(canvas_data),
						hlRawSizeX(canvas_data)*4,
						4,
						0,
						NULL );
	}
}
void undo_redo(int undo){
	if(!undo_stack)
		return;
	if(undo)
		hlUndoUndo(undo_stack);
	else
		hlUndoRedo(undo_stack);
	
}
static gboolean key_event_press(ClutterActor *a, ClutterEvent *e, gpointer data){
	guint keys = clutter_key_event_symbol(&(e->key));
	printf("key:%d\n",keys);
	switch (keys){
		case 'a':
		is_a_pressed = 1;
		break;
		case 'r':
		is_r_pressed = 1;
		break;
		case 'g':
		is_g_pressed = 1;
		break;
		case 'b':
		is_b_pressed = 1;
		break;
		case 'l':
		is_l_pressed = 1;
		break;
		case 's':
		is_s_pressed = 1;
		break;
		case 'h':
		is_h_pressed = 1;
		break;
		case 'z':
		undo_redo(0);
		break;
		case 'u':
		undo_redo(1);
		break;
	}
	return TRUE;
}
static gboolean key_event_release(ClutterActor *a, ClutterEvent *e, gpointer data){
	guint keys = clutter_key_event_symbol(&(e->key));
	printf("key:%d\n",keys);
	switch (keys){
		case 'a':
		is_a_pressed = 0;
		break;
		case 'r':
		is_r_pressed = 0;
		break;
		case 'g':
		is_g_pressed = 0;
		break;
		case 'b':
		is_b_pressed = 0;
		break;
		case 'l':
		is_l_pressed = 0;
		break;
		case 's':
		is_s_pressed = 0;
		break;
		case 'h':
		is_h_pressed = 0;
		break;
	}
	return TRUE;
}
static gboolean mouse_event_press(ClutterActor *a, ClutterEvent *e, gpointer data){
	is_mouse_pressed = 1;
	return FALSE;
}
static gboolean mouse_event_release(ClutterActor *a, ClutterEvent *e, gpointer data){
	is_mouse_pressed = 0;
	return FALSE;
}
static gboolean color_slide(ClutterActor *rect, ClutterEvent *event, gpointer data){
  	gint x = 0;
  	gint x2 = 0;
  	guint sx = 0;
	float fact;
  	clutter_event_get_coords (event, &x,NULL);
	clutter_actor_get_position(rect,&x2,NULL);
	clutter_actor_get_size(rect,&sx,NULL);

	x-=x2;
	fact = (float)x/(float)sx *255;
	if(is_r_pressed){
		paint_color.red = fact;
	}
	if(is_g_pressed){
		paint_color.green = fact;
	}
	if(is_b_pressed){
		paint_color.blue = fact;
	}
	if(is_a_pressed){
		paint_color.alpha = fact;
	}
	paint_clr = hlNewColor(cs,
				(double)paint_color.red,
				(double)paint_color.green,
				(double)paint_color.blue,
				0.0,
				(double)paint_color.alpha);
  	clutter_rectangle_set_color(CLUTTER_RECTANGLE(rect),&paint_color);
	//clutter_actor_set_rotation(CLUTTER_ACTOR(rect),CLUTTER_Z_AXIS,(float)x,128,128,0);
  	//g_print ("Rect clicked at (%d, %d)\n", x, y);
	
	return TRUE; /* Stop further handling of this event. */
}
int paint(float x, float y){
	hlColor c = hlNewColor(cs,0,0,0,0,255.0);
	hlParam *p;
	float dx = x-lastx;
	float dy = y-lasty;
	float dist = sqrtf(dx*dx + dy*dy);
	float newdist = 0;
	float addx = dx*(step/dist);
	float addy = dy*(step/dist);
	float tmpx = 0;
	float tmpy = 0;
	if(!painting){
		lastx = x;
		lasty = y;
		p = hlNewParam(HL_DRAW_RECT);
		hlParamSetColor(p,paint_clr);
		hlParamSetNum(p,x-5.0,y-5.0,x+5.0,y+5.0,1.0);
		hlImgPushNewOp(canvas_img,p);
	}else if(dist <= step){
		//printf("skip\n");
		return 0;
	}else{
		if(dist > 1000){
			return 0;
		}
	//printf("dx:%f,dy:%f,x:%f,y:%f,lastx:%f,lasty:%f\n",dx,dy,x,y,lastx,lasty);
	//printf("dist:%f,addx:%f,addy:%f\n",dist,addx,addy);
		tmpx = lastx;
		tmpy = lasty;
		do{
			tmpx += addx;
			tmpy += addy;
			p = hlNewParam(HL_DRAW_RECT);
			hlParamSetColor(p,paint_clr);
			hlParamSetNum(p,tmpx-5.0,tmpy-5.0,tmpx+5.0,tmpy+5.0,1.0);
			hlImgPushNewOp(canvas_img,p);
			//printf("newdist:%f\n",newdist);
			//printf("==================================> (%f,%f)\n",tmpx,tmpy);
			newdist += step;
		}while( newdist < dist);
		lastx = x;
		lasty = y;
	}
	return 1;
}
static gboolean paint_event(ClutterActor *rcanvas, ClutterEvent *event, gpointer data){
	gint X,Y,x,y,painted;
	clutter_event_get_coords(event,&x,&y);
	clutter_actor_get_position(rcanvas,&X,&Y);
	x -=X;
	y -=Y;
	if(is_mouse_pressed){
		//printf("paint:%d,%d\n",x,y);
		painted = paint(x,y);
		painting = 1;
	}else{
		if(painting){
			if(!undo_stack){
				undo_stack = hlNewUndo(canvas_img,32);
			}else{
				hlUndoPush(undo_stack);
			}
		}
		painting = 0;
		painted = 0;
	}
	if(painted){
		hlImgRenderToRaw(canvas_img,canvas_data,0,0,0,0);
		clutter_texture_set_from_rgb_data(	CLUTTER_TEXTURE(rcanvas),
							(guchar*)hlRawData(canvas_data),
							TRUE,
							hlRawSizeX(canvas_data),hlRawSizeY(canvas_data),
							hlRawSizeX(canvas_data)*4,
							4,
							0,
							NULL );
	}
	return TRUE;
}
static gboolean button_enter(ClutterActor *a, ClutterEvent *e,gpointer data){
	ClutterEffectTemplate *et = clutter_effect_template_new_for_duration(150,CLUTTER_ALPHA_SINE_INC);
	clutter_effect_scale(et,a,1.10,1.10,NULL,NULL);
	return TRUE;
}
static gboolean button_leave(ClutterActor *a, ClutterEvent *e,gpointer data){
	ClutterEffectTemplate *et = clutter_effect_template_new_for_duration(250,CLUTTER_ALPHA_SINE_INC);
	clutter_effect_scale(et,a,1.0,1.0,NULL,NULL);
	return TRUE;
}
static gboolean button_press_plus(ClutterActor *a, ClutterEvent *e,gpointer data){
	ClutterEffectTemplate *et = clutter_effect_template_new_for_duration(150,CLUTTER_ALPHA_SINE);
	clutter_effect_scale(et,a,0.9,0.9,NULL,NULL);
	hlRaw *Raw = hlNewRaw(cs,800,600);
	hlImgRenderToRaw(canvas_img,Raw,0,0,0,0);
	hlRawToPng(Raw,"paint.png");
	
	//rotate_rect(15);
	return TRUE;
}
static gboolean button_press_minus(ClutterActor *a, ClutterEvent *e,gpointer data){
	ClutterEffectTemplate *et = clutter_effect_template_new_for_duration(150,CLUTTER_ALPHA_SINE);
	clutter_effect_scale(et,a,0.9,0.9,NULL,NULL);
	//hlPrintImg(canvas_img,0);
	//rotate_rect(-15);
	hl_print_mem_data();
	return TRUE;
}
void add_tex_button(ClutterContainer *c,char *tex, gint x, gint y,GCallback cb){
	guint sx,sy;
	ClutterActor *button = clutter_texture_new_from_file(tex,NULL);  

	clutter_actor_get_size(button,&sx,&sy);
	clutter_container_add_actor(c,button);
	clutter_actor_set_reactive (button, TRUE);
  	clutter_actor_set_position (button,x+sx/2,y+sy/2);
	clutter_actor_set_anchor_point(button,sx/2,sy/2); 
  	g_signal_connect (button, "enter-event",G_CALLBACK (button_enter), NULL);
  	g_signal_connect (button, "leave-event",G_CALLBACK (button_leave), NULL);
	if(cb)
  		g_signal_connect (button, "button-press-event",G_CALLBACK (cb), NULL);
  	clutter_actor_show (button);
}
int main(int argc, char *argv[]){
  	ClutterColor stage_color = { 80, 80, 80, 255 }; 
  	ClutterColor canvas_color = { 50, 50, 50, 255 };
  	ClutterColor rcanvas_color = { 255, 255, 255, 255 };
  	ClutterColor canvas_border_color = { 36, 36, 36, 255 };
	ClutterActor *canvas;
	ClutterActor *tex;
	// HIMALAYA 
	cs  = hlNewCS(HL_8B,HL_RGB);
	canvas_data = hlNewRaw(cs,680,380);
	clr = hlNewColor(cs,	255.0, //(double)paint_color.red,
				255.0,//(double)paint_color.green,
				255.0,//(double)paint_color.blue,
				0.0,
				255.0 );
	hlRaw *background = hlNewRaw(cs,680,380);
	hlRawFill(background,&clr);
	
	canvas_img = hlNewImgFromSource(hlFrameFromRaw(background));
	hlImgRenderToRaw(canvas_img,canvas_data,0,0,0,0);
	
	// CLUTTER
	clutter_init (&argc, &argv);
	printf("%d\n",clutter_get_motion_events_frequency());
	clutter_set_motion_events_frequency(60);
  	// STAGE /
  	stage = clutter_stage_get_default ();
  	clutter_actor_set_size (stage, 800, 600);
  	clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);
	clutter_stage_set_user_resizable(CLUTTER_STAGE(stage),TRUE);
	
	// CANVAS /
	canvas = clutter_rectangle_new_with_color(&canvas_color);
	clutter_rectangle_set_border_color(CLUTTER_RECTANGLE(canvas),&canvas_border_color);
	clutter_rectangle_set_border_width(CLUTTER_RECTANGLE(canvas),1);
   	clutter_actor_set_size (canvas, 720, 420);
  	clutter_actor_set_position (canvas, 40, 40);
  	clutter_container_add_actor (CLUTTER_CONTAINER (stage), canvas);  
  	clutter_actor_show (canvas);
	
	//rcanvas = clutter_rectangle_new_with_color(&rcanvas_color);
	//clutter_actor_set_size(rcanvas,680,380);
	rcanvas = clutter_texture_new();
	clutter_texture_set_from_rgb_data(	CLUTTER_TEXTURE(rcanvas),
						(guchar*)hlRawData(canvas_data),
						TRUE,
						hlRawSizeX(canvas_data),hlRawSizeY(canvas_data),
						hlRawSizeX(canvas_data)*4,
						4,
						0,
						NULL );
	clutter_actor_set_position(rcanvas,60,60);
	clutter_actor_set_reactive(rcanvas,TRUE);
  	g_signal_connect (rcanvas, "motion-event",G_CALLBACK (paint_event), NULL);
  	clutter_container_add_actor (CLUTTER_CONTAINER (stage), rcanvas);  
	clutter_actor_show(rcanvas);
	
	// BUTTONS /
	add_tex_button(CLUTTER_CONTAINER(stage),"buttons/but+.png",90,480,G_CALLBACK(button_press_plus));
	add_tex_button(CLUTTER_CONTAINER(stage),"buttons/but-.png",126,480,G_CALLBACK(button_press_minus));
	add_tex_button(CLUTTER_CONTAINER(stage),"buttons/but-undo.png",172,480,NULL);
	add_tex_button(CLUTTER_CONTAINER(stage),"buttons/but-redo.png",208,480,NULL);
  	
	// COLOR PICKER /
	paint_clr = hlNewColor(cs,255.0,0,0,0,255.0);
	color_picker = clutter_rectangle_new_with_color(&paint_color);
	clutter_actor_set_size(color_picker,80,24);
	clutter_actor_set_position(color_picker,680,488);
  	clutter_container_add_actor (CLUTTER_CONTAINER (stage),color_picker);  
	clutter_actor_show(color_picker);
	clutter_actor_set_reactive (color_picker, TRUE);
	g_signal_connect (color_picker, "motion-event",G_CALLBACK (color_slide), NULL);
  	
	// STAGE: /
  	clutter_actor_show (stage);
  	g_signal_connect (stage, "key-press-event",G_CALLBACK (key_event_press), NULL);
  	g_signal_connect (stage, "key-release-event",G_CALLBACK (key_event_release), NULL);
  	g_signal_connect (stage, "button-press-event",G_CALLBACK (mouse_event_press), NULL);
  	g_signal_connect (stage, "button-release-event",G_CALLBACK (mouse_event_release), NULL);
	
  	// Start the main loop, so we can respond to events: /
  	clutter_main ();

  	return EXIT_SUCCESS;
}
