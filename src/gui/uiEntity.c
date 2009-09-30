#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "uiCore.h"
#include "uiIterator.h"

#define UI_MAX_ENTITY 1000

static int ent_count = 0;
static uiEntity *ent_over;	/*the mouse is over that entity */
static uiEntity *ent_active;	/* if not null, all events go trough that 
				   entity even if the mouse is not over.
				  */
static uiEntity *ent_screen;	/* The root entity that is currently drawn */

static void uiEntityLayout(uiEntity *e){
	uiNode *n;
	uiEntity *c;
	float px,py,sx,sy,posx,posy,sizex,sizey;
	posx = e->margin_in;
	posy = e->margin_in;
	sizex = e->sizex - 2* e->margin_in;
	sizey = e->sizey - 2* e->margin_in;
	/* those vars represent the free zone where we can align entities.
	 * that zone is shrunk when a new child entity is aligned
	 * fixed and centered entities don't take space
	 */
	px = posx;
	py = posy;
	sx = sizex;
	sy = sizey;
	if(e->child){
		n = e->child->first;
		while(n){
			c = (uiEntity*)n->data;
			/*we resize the entity to fit the available space*/
			if(c->resizable_x){
				c->sizex = sx*c->rel_sizex;
				if(c->sizex > 2*c->margin_out){
					c->sizex -= 2*c->margin_out;
				}
				c->posx = px + c->margin_out;
			}
			if(c->resizable_y){
				c->sizey = sy*c->rel_sizey;
				if(c->sizey > 2*c->margin_out){
					c->sizey -= 2*c->margin_out;
				}
				c->posy = px + c->margin_out;
			}
			/*position the entity depending on where to align */
			switch(c->align){
				case UI_ALIGN_NORTH:
					c->posy = py + sy - c->sizey - 2*c->margin_out;
					sy -= c->sizey + 2*c->margin_out;
					break;
				case UI_ALIGN_EAST:
					c->posx = px + sx - c->sizex - 2*c->margin_out;
					sx -= c->sizex + 2*c->margin_out;
					break;
				case UI_ALIGN_SOUTH:
					c->posy = py + c->margin_out;
					sy -= c->sizey + 2*c->margin_out;
					break;
				case UI_ALIGN_WEST:
					c->posx = px + c->margin_out;
					sx -= c->sizex + 2*c->margin_out;
					break;
				default: 
					/*TODO UI_ALIGN_CENTER*/
					break;
			}
			uiEntityLayout(c);
			n = n->next;
		}
	}
}
void uiEntityLayoutAll(){
	if(ent_screen){
		ent_screen->sizex = uiWindowGetSizeX();
		ent_screen->sizey = uiWindowGetSizeY();
		ent_screen->posx = 0;
		ent_screen->posy = 0;
		uiEntityLayout(ent_screen);
	}
}
static void uiEntityDraw(uiEntity *e){
	float x,y;
	uiNode *n;
	glPushMatrix();
	glTranslatef(e->posx,e->posy,1.0f);
	if(e->draw){
		e->draw(e);
	}
	if(e->type == UI_ENT_REGION){
		if(e->child){
			x = uiEntityGetPosX(e);
			y = uiEntityGetPosY(e);
			n = e->child->first;
			while(n){
				glScissor(x,y,e->sizex,e->sizey);
				glEnable(GL_SCISSOR_TEST);
				uiEntityDraw((uiEntity*)n->data);
				glDisable(GL_SCISSOR_TEST);
				n = n->next;
			}
		}
	}else{
		if(e->child){
			n = e->child->first;
			while(n){
				uiEntityDraw((uiEntity*)n->data);
				n = n->next;
			}
		}
	}
	glPopMatrix();
}
void uiEntityDrawAll(){
	if(ent_screen){
		uiEntityDraw(ent_screen);
	}
}
void uiEntityAdd(uiEntity*ent, uiEntity*parent){
	if(!parent->child){
		parent->child = uiListNew();
	}
	ent->parent = parent;
	uiListAdd(parent->child,ent);
	ent->visible = 1;
	ent_count++;
}
void uiScreenSet(uiEntity *ent){
	if(ent->type == UI_ENT_SCREEN){
		ent_screen = ent;
		ent->alive = 1;
		ent->visible = 1;
	}else{
		printf("WARNING : Cannot set non-screen entity as current screen\n");
	}
}
static void uiEntityClean(uiEntity *ent){
	uiNode *n;
	if(ent->child){
		n = ent->child->first;
		while(n){
			uiEntityClean((uiEntity*)n->data);
			n = n->next;
		}
	}
	if(!ent->alive){	/*TODO this is very ugly */
		if(ent->parent && ent->parent->child){
			uiListRemove(ent->parent->child,ent);
		}
		uiEntityFree(ent);
		ent_count--;
	}
}
		
void uiEntityCleanAll(void){
	if(ent_screen){
		uiEntityClean(ent_screen);
	}
}
uiEntity * uiEntityNew(const char *name, int type ){
	uiEntity *ent = (uiEntity*)malloc(sizeof(uiEntity));
	memset(ent,0,sizeof(uiEntity));
	ent->type = type;
	strncpy(ent->name,name,UI_NAME_LENGTH);
	ent->alive = 1;
	ent->parent = NULL;
	ent->posx = 0.0;
	ent->posy = 0.0;
	ent->sizex = 32.0;
	ent->sizey = 24.0;
	ent->posz = 0.0;
	return ent;
}
float uiEntityGetPosX(uiEntity *ent){
	if(ent->parent == NULL){
		return ent->posx;
	}else{
		return uiEntityGetPosX(ent->parent) + ent->posx;
	}
}
float uiEntityGetPosY(uiEntity *ent){
	if(ent->parent == NULL){
		return ent->posy;
	}else{
		return uiEntityGetPosY(ent->parent) + ent->posy;
	}
}
float uiEntityGetPosZ(uiEntity *ent){
	if(ent->parent == NULL){
		return ent->posz;
	}else{
		return uiEntityGetPosZ(ent->parent) + ent->posz;
	}
}
void	uiEntitySetPos(uiEntity *ent, float posx, float posy){
	ent->posx = posx;
	ent->posy = posy;
}
void 	uiEntitySetSize(uiEntity *ent, float sizex, float sizey){
	ent->sizex = sizex;
	ent->sizey = sizey;
}
void  uiEntityMakeTexture(uiEntity *ent, int sizex, int sizey){
	if(ent->tex){
		if(ent->tex_sizex == sizex && ent->tex_sizey == sizey){
			return;
		}else{
			uiEntityFreeTexture(ent);
		}
	}
	ent->tex_sizex = sizex;
	ent->tex_sizey = sizey;
	ent->tex = malloc(sizex*sizey*4);
}
void  uiEntitySetTexture(uiEntity *ent, char *texture, int sizex, int sizey){
	uiEntityFreeTexture(ent);
	ent->tex = texture;
	ent->tex_sizex = sizex;
	ent->tex_sizey = sizey;
}
void  uiEntitySetTexturePos(uiEntity*ent, float posx, float posy, float posz,
			 	float sizex, float sizey){
	ent->tex_posx = posx;
	ent->tex_posy = posy;
	ent->tex_dsizex = sizex;
	ent->tex_dsizey = sizey;
}
char *uiEntityGetTexture(uiEntity *ent, int *sizex, int *sizey){
	if(sizex){
		*sizex = ent->tex_sizex;
	}
	if(sizey){
		*sizey = ent->tex_sizey;
	}
	return ent->tex;
}
void   uiEntityFreeTexture(uiEntity *ent){
	if(ent->tex){
		free(ent->tex);
	}
}
void    uiEntityFree(uiEntity *ent){
	if(ent->data){
		free(ent->data);
	}
	free(ent);
}
static int onEntity(uiEntity *ent, float posx, float posy){
	float x = uiEntityGetPosX(ent);
	float y = uiEntityGetPosY(ent);
	if( 	x <= posx && y <= posy
		&& x + ent->sizex > posx
		&& y + ent->sizey > posy ){
		return 1;
	}else{
		return 0;
	}
}
/*picks the closest entity that */
static uiEntity *uiEntityPickEnt(uiEntity *ent, float posx, float posy){
	uiNode *n = NULL;
	uiEntity *tmp = NULL;
	if(!ent || !ent->visible || !onEntity(ent,posx,posy)){
		return NULL;
	}
	if(!ent->child){
		return ent;
	}
	n = ent->child->first;
	while(n){	/*if a child can be picked we return that child */
		if((tmp = uiEntityPickEnt((uiEntity*)n->data,posx,posy))){
			return tmp;
		}
		n = n->next;
	}
	return ent;
}
uiEntity *uiEntityPick(float posx, float posy){
	return uiEntityPickEnt(ent_screen,posx,posy);
}
void uiEventMouseButton(int button, int down, float x, float y, float pressure){
	uiEntity *ent = uiEntityPick(x,y);
	printf("click...\n");
	while(ent){
		printf("ent: %s\n",ent->name);
		if(ent->click){
			if(!ent->click(ent,button,down,x,y,pressure)){
				return;
			}
		}
		ent = ent->parent;
	}
}
static void uiEntitySetMouseOver(uiEntity *ent, int value){
	while(ent){
		ent->mouseover = value;
		ent = ent->parent;
	}
}
void uiEventMouseMotion(float x, float y, float pressure){
	uiEntity *ent = uiEntityPick(x,y);
	/* if an entity is beeing dragged, it grabs the focus.
	 */
	if((uiStateMouse(UI_MOUSE_BUTTON_1) || uiStateMouse(UI_MOUSE_BUTTON_2))
			&& ent_active && ent_active->motion){
		uiEntitySetMouseOver(ent_active,1);
		ent_active->motion(ent_active,x,y,pressure);
		return;
	}
	/*there is no dragging : we remove the active entity, and change
	 * the mouse over status*/
	if(ent_active){
		uiEntitySetMouseOver(ent_active,0);
		ent_active = NULL;
	}
	if(ent != ent_over){
		uiEntitySetMouseOver(ent_over,0);
		ent_over = ent;
	}
	uiEntitySetMouseOver(ent,1);

	/* sending mouse over event*/
	while(ent){
		if(ent->motion){
			if(!ent->motion(ent,x,y,pressure)){
				ent_active = ent;
				return;
			}
		}
		ent = ent->parent;
	}
}

