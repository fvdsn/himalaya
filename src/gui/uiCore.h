#ifndef __UI_CORE_H__
#define __UI_CORE_H__

#define UI_NAME_LENGTH 32
#define UI_FRAME_INTERVAL 20

typedef long   uiTime;
typedef struct uiEvent_s uiEvent;
typedef struct uiEntity_s uiEntity;
typedef struct uiState_s uiState;

enum ui_entity_type{
	UI_ENT_WINDOW,
	UI_ENT_PANEL,
	UI_ENT_BUTTON,
	UI_ENT_SLIDER,
	UI_ENT_COLOR,
	UI_ENT_CANVAS
};

#define ALIGN_CENTER 0
#define ALIGN_LEFT -1
#define ALIGN_RIGHT 1
#define ALIGN_TOP 1
#define ALIGN_BOTTOM -1

struct uiEntity_s{
	int type;
	char name[UI_NAME_LENGTH];
	int alive;

	/* callbacks */
	uiEntity *parent;
	void (*draw)(uiEntity *self);
	int (*event)(uiEntity *self, uiEvent *event);
	int (*click)(uiEntity *self, int button, int down, float x, float y, float pressure);
	int (*motion)(uiEntity *self,float x, float y, float pressure);
	
	/* drawing */
	int visible;
	int active;
	float posx;
	float posy;
	float posz;
	float sizex;
	float sizey;

	/*flow and alignment*/
	float margin_out;	/* outer margin in pixel */
	float margin_in;	/* inner margin in pixel */
	int align;		/* the particle align itselfs */
	int align_x;		/* horizontal alignement */
	int align_y;		/* vertical alignment    */
	uiEntity *flow;		/* if not null the entity will 
				   align to this one */

	/*texture*/
	float  	tex_posx;	/*display offset of the texture */
	float  	tex_posy;
	float   tex_posz;
	float   tex_dsizex;	/*display size of the texture */
	float   tex_dsizey;
	int 	tex_sizex;	/*internal size of the texture */
	int	tex_sizey;
	char *tex;

	/*state*/
	int mouseover;		/*the mouse is over, or dragging from the entity*/
	long mouse_over_time;
	long mouse_leave_time;
	long mouse_click_time;

	void *data;
};

uiEntity * uiEntityNew(const char *name, int type, uiEntity *parent);
void	uiEntitySetPos(uiEntity *ent, float posx, float posy);
void 	uiEntitySetSize(uiEntity *ent, float sizex, float sizey);
void    uiEntityFree(uiEntity *ent);
void 	uiEntityCleanAll(void);
void 	uiEntityDrawAll(void);
void 	uiEntityAdd(uiEntity*ent);
float 	uiEntityGetPosX(uiEntity *ent);
float 	uiEntityGetPosY(uiEntity *ent);
float 	uiEntityGetPosZ(uiEntity *ent);
float 	uiEntityGetSizeX(uiEntity *ent);
float 	uiEntityGetSizeY(uiEntity *ent);

void  	uiEntityMakeTexture(uiEntity *ent, int sizex, int sizey);
void  	uiEntitySetTexture(uiEntity *ent, char *texture, int sizex, int sizey);
void  	uiEntitySetTexturePos(uiEntity*ent, float posx, float posy, float posz,
			 	float sizex, float sizey);
char*	uiEntityGetTexture(uiEntity *ent, int *sizex, int *sizey);
void   	uiEntityFreeTexture(uiEntity *ent);

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

#endif

