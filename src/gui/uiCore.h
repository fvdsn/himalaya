#ifndef __UI_CORE_H__
#define __UI_CORE_H__
#include "uiIterator.h" 

#define UI_NAME_LENGTH 32
#define UI_STRING_LENGTH (UI_NAME_LENGTH*32)
#define UI_FRAME_INTERVAL 20

typedef long   uiTime;
typedef struct uiEvent_s uiEvent;
typedef struct uiEntity_s uiEntity;
typedef struct uiState_s uiState;

enum ui_entity_type{
	UI_ENT_SCREEN,
	UI_ENT_RECT,
	UI_ENT_PANEL,
	UI_ENT_BUTTON,
	UI_ENT_SLIDER,
	UI_ENT_COLOR,
	UI_ENT_CANVAS,
	UI_ENT_LABEL,
	UI_ENT_CAIROLABEL,
	UI_ENT_REGION,
	UI_ENT_TABENV,
	UI_ENT_DIV,
	UI_ENT_HL
};

enum ui_align{
	UI_ALIGN_FIXED,
	UI_ALIGN_CENTER,
	UI_ALIGN_NORTH,
	UI_ALIGN_EAST,
	UI_ALIGN_SOUTH,
	UI_ALIGN_WEST
};
enum ui_margin{
	UI_MARGIN_ALL,
	UI_MARGIN_NORTH,
	UI_MARGIN_EAST,
	UI_MARGIN_SOUTH,
	UI_MARGIN_WEST
};

typedef struct ui_string{
	char text[UI_STRING_LENGTH]; /*the text displayed*/
	const char*font;
	float color[4];
	float font_size;
	int font_weight;
	int font_slant;
	float px;
	float py;
	int sx;		/*size of the buffer*/
	int sy;
	char *buffer;	/* the texture rendered */
	int uptodate;	/*if zero : the string needs to be redrawn*/
}uiString;

struct uiEntity_s{
	char name[UI_NAME_LENGTH];
	int type;
	int subtype;
	int uid;
	int alive;	/*if not it should be freed asap*/
	int active;	/* TODO if is the active entity */
	int display;	/* 0 do not draw or send event  */
	int enabled;
	uiEntity *parent;
	uiList *child;
	uiString *namestring;

	/* callbacks */
	void (*draw)(uiEntity *self);
	int (*event)(uiEntity *self, uiEvent *event);
	int (*click)(uiEntity *self, int button, int down, float x, float y, float pressure);
	int (*motion)(uiEntity *self,float x, float y, float pressure);
	
	/* drawing */
	int visible;
	float posx;
	float posy;
	float posz;
	float sizex;
	float sizey;
	float inner_sizex;
	float inner_sizey;
	float dx;	/*positionning of the child widgets*/
	float dy;
	float color[4];

	/*flow and alignment*/
	float margin_out;	/* outer margin in pixel */
	float margin_in_north;
	float margin_in_east;
	float margin_in_south;
	float margin_in_west;
	
	int resizable_x;
	float rel_sizex;
	
	int resizable_y;
	float rel_sizey;

	int align; /* N,E,S,W */

	/*string*/
	uiString *string;

	/*state*/
	int mouseover;		/*the mouse is over, or dragging from the entity*/
	long mouse_over_time;
	long mouse_leave_time;
	long mouse_click_time;

	void *data;
};

uiEntity * uiEntityNew(const char *name, int type);
void	uiEntitySetPos(uiEntity *ent, float posx, float posy);
void 	uiEntitySetSize(uiEntity *ent, float sizex, float sizey);
void    uiEntityFree(uiEntity *ent);
void 	uiEntityCleanAll(void);
void 	uiEntityDrawAll(void);
void 	uiEntityLayoutAll(void);
void 	uiEntityAdd(uiEntity*ent,uiEntity *parent);
float 	uiEntityGetPosX(uiEntity *ent);
float 	uiEntityGetPosY(uiEntity *ent);
float 	uiEntityGetPosZ(uiEntity *ent);
float 	uiEntityGetSizeX(uiEntity *ent);
float 	uiEntityGetSizeY(uiEntity *ent);
void	uiEntityMousePos(uiEntity *ent, float *x, float *y,float *pressure);
void	uiEntityAlign(uiEntity *ent, enum ui_align dir);
void 	uiEntityFitX(uiEntity *ent, float relsize);
void 	uiEntityFitY(uiEntity *ent, float relsize);
void	uiEntitySetMargin(uiEntity *ent, enum ui_margin side, float px);
void	uiScreenSet(uiEntity *ent);

uiEntity * uiEntityPick(float posx, float posy);

struct uiState_s{
	char name[UI_NAME_LENGTH];
	int  value;
	uiEntity *ent;
};

#define UI_KEY_UP 0
#define UI_KEY_DOWN 1

enum ui_mod_key{
	UI_CTRL_L,
	UI_CTRL_R,
	UI_SHIFT_L,
	UI_SHIFT_R,
	UI_ALT,
	UI_SPACE,
	UI_MOD_KEY_COUNT
};
enum ui_mouse_button{
	UI_MOUSE_BUTTON_1,
	UI_MOUSE_BUTTON_2,
	UI_MOUSE_BUTTON_3,
	UI_MOUSE_BUTTON_COUNT
};
void uiStateSetValue(const char *name, int down);
void uiStateSetEnt(const char *name, uiEntity *ent);
int  uiStateGetValue(const char *name);
uiEntity* uiStateGetEnt(const char *name);
void uiStateSetKey(char key, int down);
void uiStateSetMod(int mod, int down);
void uiStateSetMouse(int button, int down);
void uiStateSetMousePos(float x, float y,float pressure);
int  uiStateKey(char key);
int  uiStateMod(int Mod);
int  uiStateMouse(int button);
void  uiStateMousePos(float *x, float *y,float *pressure);
void  uiStateMouseDelta(float *dx, float *dy,float *dp);

struct uiEvent_s{
	int type;
	uiEntity * source;
	int channel;
	int datasize;
	void *data;
};

void uiEventSend(uiEntity *src, int channel, int type, int datasize, void *data);
void uiEventMouseButton(int button, int down , float x, float y, float pressure);
void uiEventMouseMotion(float x, float y, float pressure);
void uiEventMouseDrag(int button, float x, float y, float dx, float dy, float p1, float p2);
void uiEventListen(uiEntity *ent, int channel, int type );
void uiEventForget(uiEntity *ent);
void uiEventProcess(void);
void uiEventCallback(int channel, int type, int (*callback)(uiEvent *e));

enum color_type{
	UI_BG_COLOR,
	UI_FONT_COLOR,
	UI_ITEM_COLOR,
	UI_PANEL_COLOR,
	UI_COLOR_TYPE_COUNT
};
enum color_modifier{
	UI_NORMAL_COLOR,
	UI_ACTIVE_COLOR,
	UI_COLOR_MODIFIER_COUNT
};
float *uiWindowGetColor(int type, int modifier);
int  uiWindowGetSizeX(void);
int  uiWindowGetSizeY(void);
void uiWindowDrawBegin(void);
void uiWindowDrawEnd(void);
void uiNewWindow(const char *name, int sizex, int sizey);
void uiMainLoop(void);


uiString *uiStringNew(const char *text, float font_size, int sx, int sy, float px, float py);
void uiStringDraw(uiString *s, float px, float py, float pz, float sx, float sy);
void uiStringSetSize(uiString *s, float sx, float sy);


#endif

