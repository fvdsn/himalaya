#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include "uiCore.h"

#define UI_MAX_ENTITY 1000

static int ent_count;
static uiEntity *ent_list[UI_MAX_ENTITY];
static uiEntity *ent_active;

static int entity_z_compare(const void *a, const void *b){
	uiEntity *ea = *(uiEntity**)a;
	uiEntity *eb = *(uiEntity**)b;
	float za = uiEntityGetPosZ(ea);
	float zb = uiEntityGetPosZ(eb);
	if( za < zb){
		return -1;
	}else if (za == zb){
		return 0;
	}else{
		return 1;
	}
}

void uiEntityDrawAll(void){
	int i = 0;
	qsort(ent_list,ent_count,sizeof(uiEntity*),entity_z_compare);
	while (i < UI_MAX_ENTITY){
		if(ent_list[i] && ent_list[i]->alive && ent_list[i]->draw){
			ent_list[i]->draw(ent_list[i]);
		}
		i++;
	}
}
void uiEntityAdd(uiEntity*ent){
	int i = 0;
	if(ent_count >= UI_MAX_ENTITY){
		printf("ERROR : ui_entity_add() : max ui entity count reached\n");
	
	}
	while(i < UI_MAX_ENTITY){
		if(!ent_list[i]){
			ent_list[i] = ent;
			ent_count++;
			return;
		}
		i++;
	}
}
void uiEntityCleanAll(void){
	int i = 0;
	while(i < UI_MAX_ENTITY){
		if(ent_list[i] && !ent_list[i]->alive){
			uiEntityFree(ent_list[i]);
			ent_list[i] = NULL;
			ent_count--;
		}
		i++;
	}
}
uiEntity * uiEntityNew(const char *name, int type, uiEntity *parent){
	uiEntity *ent = (uiEntity*)malloc(sizeof(uiEntity));
	memset(ent,0,sizeof(uiEntity));
	ent->type = type;
	strncpy(ent->name,name,UI_NAME_LENGTH);
	ent->alive = 1;
	if(parent){
		ent->parent = parent;
		ent->posx = 0;
		ent->posy = 0;
		ent->sizex = 32.0;
		ent->sizey = 24.0;
		ent->posz =  1.0;
	}else{
		ent->parent = NULL;
		ent->posx = 0.0;
		ent->posy = 0.0;
		ent->sizex = 32.0;
		ent->sizey = 24.0;
		ent->posz = 0.0;
	}
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
uiEntity *uiEntityPick(float posx, float posy){
	int i = UI_MAX_ENTITY;
	uiEntity *ent;
	while(i--){
		if((ent = ent_list[i]) && onEntity(ent,posx,posy)){
			return ent;
		}
	}
	return NULL;
}
void uiEventMouseButton(int button,int down, float x, float y, float pressure){
	int i = UI_MAX_ENTITY;
	uiEntity *ent;
	while(i--){
		if( (ent = ent_list[i]) && onEntity(ent,x,y) && ent->click){
			if(!ent->click(ent,button,down,x,y,pressure)){
				return;
			}
		}
	}
}
void uiEventMouseMotion(float x, float y, float pressure){
	int i = UI_MAX_ENTITY;
	uiEntity *ent;
	if(uiStateMouse(UI_MOUSE_BUTTON_1) || uiStateMouse(UI_MOUSE_BUTTON_2)){
		if(ent_active && ent_active->motion){
			ent_active->motion(ent_active,x,y,pressure);
			return;
		}
	}else{
		ent_active = NULL;
	}
	while(i--){
		if( (ent = ent_list[i])){
			if(onEntity(ent,x,y)){
				ent->mouseover = 1;
				if(ent->motion){
					if(!ent->motion(ent,x,y,pressure)){
						ent_active = ent;
						return;
					}
				}
			}else{
				ent->mouseover = 0;
			}
		}
	}
}



