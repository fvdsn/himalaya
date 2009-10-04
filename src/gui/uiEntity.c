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
static void uiEntityInnerTighten(uiEntity *e){
	uiNode *n;
	uiEntity *c;
	float sx = e->margin_in_west  + e->margin_in_east;
	float sy = e->margin_in_north + e->margin_in_south;
	float px = 0;
	float py = 0;
	if(!e->child){
		e->inner_sizex = e->sizex;
		e->inner_sizey = e->sizey;
	}else{
		n = e->child->first;
		while(n){
			c = (uiEntity*)n->data;
			if(c->align == UI_ALIGN_FIXED){
				if(px < c->posx + c->sizex + e->margin_in_east){
					px = c->posx + c->sizex + e->margin_in_east;
				}
				if(py < c->posy + c->sizey + e->margin_in_north){
					py = c->posy + c->sizey + e->margin_in_north;
				}
			}else{
				sx += c->sizex;
				sy += c->sizey;
			}
			n = n->next;
		}
		if(sx < px){ sx = px;}
		if(sy < py){ sy = py;}
		if(sx < e->sizex){
			e->inner_sizex = e->sizex;
		}else{
			e->inner_sizex = sx;
		}
		if(sy < e->sizey){
			e->inner_sizey = e->sizey;
		}else{
			e->inner_sizey = sy;
		}
	}
}
		


static void uiEntityLayout(uiEntity *e){
	uiNode *n;
	uiEntity *c;
	float px,py,sx,sy,posx,posy,sizex,sizey;
	posx = e->margin_in_west;
	posy = e->margin_in_south;
	if(e->type == UI_ENT_REGION){
		uiEntityInnerTighten(e);
		sizex = e->inner_sizex - e->margin_in_west - e->margin_in_east;
		sizey = e->inner_sizey - e->margin_in_south - e->margin_in_north;
	}else{
		sizex = e->sizex - e->margin_in_west - e->margin_in_east;
		sizey = e->sizey - e->margin_in_south - e->margin_in_north;
	}
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
			if(!c->display){
				n = n->next;
				continue;
			}
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
				c->posy = py + c->margin_out;
			}
			/*position the entity depending on where to align */
			switch(c->align){
				case UI_ALIGN_NORTH:
					c->posy = py + sy - c->sizey - 1*c->margin_out;
					sy -= c->sizey + 1*c->margin_out;
					break;
				case UI_ALIGN_EAST:
					c->posx = px + sx - c->sizex - 1*c->margin_out;
					sx -= c->sizex + 1*c->margin_out;
					break;
				case UI_ALIGN_SOUTH:
					c->posy = py + c->margin_out;
					sy -= c->sizey + 1*c->margin_out;
					py += c->sizey + 1*c->margin_out;
					break;
				case UI_ALIGN_WEST:
					c->posx = px + c->margin_out;
					sx -= c->sizex + 1*c->margin_out;
					px += c->sizex + 1*c->margin_out;
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
	if(!e->display){
		return;
	}
	glPushMatrix();
	glTranslatef(e->posx,e->posy,1.0f);
	if(e->draw){
		e->draw(e);
	}
	glTranslatef(e->dx,e->dy,0.0f);
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
	ent->active = 1;
	ent->display = 1;
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
		return uiEntityGetPosX(ent->parent) + ent->posx + ent->parent->dx;
	}
}
float uiEntityGetPosY(uiEntity *ent){
	if(ent->parent == NULL){
		return ent->posy;
	}else{
		return uiEntityGetPosY(ent->parent) + ent->posy + ent->parent->dy;
	}
}
float uiEntityGetPosZ(uiEntity *ent){
	if(ent->parent == NULL){
		return ent->posz;
	}else{
		return uiEntityGetPosZ(ent->parent) + ent->posz;
	}
}
void	uiEntityMousePos(uiEntity *ent, float *x, float *y,float *pressure){
	float mx,my;
	uiStateMousePos(&mx,&my,pressure);
	if(x){
		*x = mx - uiEntityGetPosX(ent);
	}
	if(y){
		*y = my - uiEntityGetPosY(ent);
	}
}
void	uiEntityAlign(uiEntity *ent, enum ui_align dir){
	ent->align = dir;
}
void 	uiEntityFitX(uiEntity *ent, float relsize){
	ent->resizable_x = 1;
	if(relsize < 0 || relsize > 1){
		ent->rel_sizex = 1;
	}else{
		ent->rel_sizex = relsize;
	}
}
void 	uiEntityFitY(uiEntity *ent, float relsize){
	ent->resizable_y = 1;
	if(relsize < 0 || relsize > 1){
		ent->rel_sizey = 1;
	}else{
		ent->rel_sizey = relsize;
	}
}
void	uiEntitySetMargin(uiEntity *ent, enum ui_margin side, float px){
	switch(side){
		case UI_MARGIN_NORTH:
			ent->margin_in_north = px;
			break;
		case UI_MARGIN_EAST:
			ent->margin_in_east  = px;
			break;
		case UI_MARGIN_SOUTH:
			ent->margin_in_south = px;
			break;
		case UI_MARGIN_WEST:
			ent->margin_in_west  = px;
			break;
		default:
			ent->margin_in_north = px;
			ent->margin_in_east  = px;
			ent->margin_in_south = px;
			ent->margin_in_west  = px;
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
void    uiEntityFree(uiEntity *ent){
	if(ent->data){
		free(ent->data);
	}
	free(ent);
	/*TODO free strings and childs */
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
			if(!ent->click(ent,button,down,
					x - uiEntityGetPosX(ent),
					y - uiEntityGetPosY(ent),
					pressure)){
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

