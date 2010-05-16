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
typedef struct ui_action_s uiAction;

/**
 * Type of the entities, if you want to create a new widget,
 * it is best to add your type here
 */
enum ui_entity_type{
	UI_ENT_NONE,
	UI_ENT_ANY,
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
	UI_ENT_DISPLAY,
	UI_ENT_HL,
	UI_ENT_COUNT
};
/**
 * The entities align directions, relative to parent
 */
enum ui_align{
	UI_ALIGN_FIXED,		/*keep current position*/
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
	char text[UI_STRING_LENGTH]; /*the text to be displayed*/
	char drawntext[UI_STRING_LENGTH]; /*the text displayed*/
	const char*font;
	float color[4];
	float font_size;
	int font_weight;
	int font_slant;
	float px;	/*position of the text in the buffer ? TODO document this */
	float py;
	int sx;		/*size of the buffer*/
	int sy;
	float values[4];
	int valcount;
	char *buffer;	/* the texture rendered */
}uiString;

struct uiEntity_s{
	char name[UI_NAME_LENGTH];
	int type;	
	int alive;	/*if not it should be freed asap*/
	int active;	/* TODO if is the active entity */
	int display;	/* 0 do not draw or send event  */
	int enabled;	/* 0 : draw grayed out, do not send event */
	uiEntity *parent;
	uiList *child;
	uiString *namestring;	/*string with the name of the entity */

	/* callbacks */
	void (*draw)(uiEntity *self);	/*called when drawing */
	int (*event)(uiEntity *self, uiEvent *event);	/*called when receiving an event */
	int (*click)(uiEntity *self, int button, int down, float x, float y, float pressure); /*called when the mouse clicks*/
	int (*motion)(uiEntity *self,float x, float y, float pressure);/*called when the mouse moves over the entity*/
	int (*key_press)(uiEntity *self,int key, int down);
	
	/* drawing */
	int visible;
	float posx;		/*position relative to parent in pixels. X is right, Y is up */
	float posy;
	float posz;
	float sizex;		/*size of the entity in pixels */
	float sizey;
	float inner_sizex;	/*for region widgets : description of the inside area */
	float inner_sizey;
	float inner_posx;
	float inner_posy;
	float dx;		/*positionning of the child widgets*/
	float dy;
	float view_sizex;	/*the part of the entity that is displayed in [0,sizex]*/
	float view_sizey;
	float view_posx; 	/*in [0,sizex]*/
	float view_posy;
	float color[4];		/* for various uses */

	/*flow and alignment*/
	float margin_out;	/* outer margin in pixel */
	float margin_in_north;
	float margin_in_east;
	float margin_in_south;
	float margin_in_west;
	
	int resizable_x;	/* 0 : do not resize or fit this entity */
	float rel_sizex;	/* the ratio of parent entity size to this entity's size */
	
	int resizable_y;
	float rel_sizey;

	int align; /* N,E,S,W */

	/*string*/
	uiString *string;

	/*state*/
	int mouseover;		/*the mouse is over, or dragging from the entity*/
	long mouse_over_time;	/*TODO not done yet */
	long mouse_leave_time;
	long mouse_click_time;
	
	/* for the entity's private data */
	void *data;
};

/* for events callback, define if we need to continue sending the event
 * to callbacks with lower priority
 */
#define UI_DONE 0	
#define UI_CONTINUE 1

/** Allocates memory for a new entity.
 * @param name  the name of the entity (max UI_NAME_LENGTH)
 * @param type	the type of the new entity (see enum ui_entity_type)
 * @return a new entity with all fields other than name and type set to 0 or
 * NULL
 */
uiEntity * uiEntityNew(const char *name, int type);
/** Changes the position of the bottom left corner of the entity, relative to
 * the parent entity position.
 * @param ent The entity, must not be null
 * @param posx the horizontal position in pixels relative to parent entity. right is
 * postive.
 * @param posy the vertical position in pixels relative to parent entity, up is positive.
 */
void	uiEntitySetPos(uiEntity *ent, float posx, float posy);
/** Changes the size of the entity.
 * @param ent The entity, must not be null
 * @param posx the horizontal size in pixels of the entity. ( >0.0 )
 * @param posy the vertical size in pixels of entity, ( >0.0 )
 */
void 	uiEntitySetSize(uiEntity *ent, float sizex, float sizey);
/** Changes the margin of the entity.
 * @param ent The entity, must not be null
 * @param size the size of the margin, see enum ui_margin
 * @param margin The space between this entity and other entities, or the
 * parent entity bounding box, in pixels. ( >= 0.0 )
 */
void	uiEntitySetMargin(uiEntity *ent, enum ui_margin side, float margin);
/** Frees the entity and it's private data
 * TODO : create a free(ent) callback to free local data.
 * @param : the entity, must not be NULL.
 */
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
void	uiScreenSet(uiEntity *ent);

uiEntity * uiEntityPick(float posx, float posy);
uiEntity * uiEntityGetActive(void);

struct uiState_s{
	char name[UI_NAME_LENGTH];
	int  value;
	uiEntity *ent;
};

#define UI_KEY_UP 0
#define UI_KEY_DOWN 1

#define UI_KEY_PRESS 	0
#define UI_KEY_HOLD 	1
#define UI_KEY_RELEASE 	2 

enum ui_mod_key{
	UI_MOD_NONE	= 0,
	UI_CTRL_L 	= 1,
	UI_CTRL_R 	= 2,
	UI_CTRL		= 4,
	UI_SHIFT_L 	= 8,
	UI_SHIFT_R	= 16,
	UI_SHIFT	= 32,
	UI_ALT_L	= 64,
	UI_ALT_R	= 128,
	UI_ALT		= 256,
	UI_SPACE	= 512,
	UI_MOD_ANY	= ~0
};
enum ui_mouse_button{
	UI_MOUSE_BUTTON_1,
	UI_MOUSE_BUTTON_2,
	UI_MOUSE_BUTTON_3,
	UI_MOUSE_BUTTON_ANY,
	UI_MOUSE_BUTTON_NONE,
	UI_MOUSE_BUTTON_COUNT
};
void uiStateProcess(void);
void uiStateSetValue(const char *name, int value);
void uiStateSetEnt(const char *name, uiEntity *ent);
int  uiStateGetValue(const char *name);
uiEntity* uiStateGetEnt(const char *name);
void uiStateSetKey(int key, int down);
void uiStateSetMod(int mod, int down);
void uiStateSetMouse(int button, int down);
void uiStateSetMousePos(float x, float y,float pressure);
int  uiStateKey(int key);
int  uiStateMod(int Mod);
int  uiStateMouse(int button);
int  uiStateKeyStatus(int key);
int  uiStateMouseStatus(int button);
void  uiStateMousePos(float *x, float *y,float *pressure);
void  uiStateMouseDelta(float *dx, float *dy,float *dp);

void uiEventMouseButton(int button, int down , float x, float y, float pressure);
void uiEventMouseMotion(float x, float y, float pressure);
void uiEventMouseDrag(int button, float x, float y, float dx, float dy, float p1, float p2);
void uiEventKeyPress(int key, int down);

void uiDrawCircle(float x, float y, float z, float r);
void uiDrawDisc(float x, float y, float z, float r);
void uiRectDraw(float x, float y, float z,float sx, float sy);

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
char *uiStringWrite(uiString *s);
void uiStringSetColor(uiString *s, float r, float g, float b, float a);


#endif

