#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cairo.h>
#include "uiCore.h"
#include "uiWidget.h"

#define UI_TAB_HEIGHT 16
#define UI_TAB_SPACING 2


typedef struct ui_tabenv_data{
	int display_index;	/*index of the displayed tab*/
}uiTabEnvData;
static int uiTabCount(uiEntity *tabenv){
	return tabenv->child->count;
}
static void uiTabEnvDraw(uiEntity *self){
	uiNode *n = self->child->first;
	uiEntity *tab = NULL;
	int tabcount = uiTabCount(self);
	int index    = 0;
	int display_index = ((uiTabEnvData*)self->data)->display_index;
	float tabwidth;
	glColor4f(0.45,0.45,0.45,1);
	uiRectDraw(0,0,0,self->sizex, self->sizey - UI_TAB_HEIGHT - 2*UI_TAB_SPACING);
	if(!tabcount){return;}
	tabwidth = (self->sizex)/tabcount;
	while(n){
		float x = index *tabwidth + UI_TAB_SPACING/2;
		float y = self->sizey - UI_TAB_HEIGHT - UI_TAB_SPACING;
		float sx = tabwidth - UI_TAB_SPACING;
		float sy = UI_TAB_HEIGHT;
		tab = (uiEntity*)(n->data);
		glColor4f(0.40,0.40,0.40,1);
		if(index == display_index){
			glColor4f(0.45,0.45,0.45,1);
			y -= UI_TAB_SPACING;
			sy += 2*UI_TAB_SPACING;
		}
		if(index == tabcount - 1){
			sx += UI_TAB_SPACING/2;
		}
		if(index == 0){
			x -= UI_TAB_SPACING/2;
			sx += UI_TAB_SPACING/2;
		}
		uiRectDraw(x,y,0,sx,sy);
		if(tab && tab->namestring){
			uiStringDraw(tab->namestring,x,y,0.1,sx,sy);
		}
		n = n->next;
		index++;
	}
}
static int uiTabEnvClick(uiEntity *self,int button, int down, float x, float y, float pressure){
	uiTabEnvData *ted = (uiTabEnvData*)self->data;
	int tabcount = uiTabCount(self);
	if(tabcount && button == 0 && down == UI_KEY_UP &&
			y > self->sizey - UI_TAB_HEIGHT - UI_TAB_SPACING ){
		int tab = x/self->sizex*tabcount;
		int i = 0;
		uiNode *n = self->child->first;
		if(tab == tabcount){
			tab --;
		}
		while(n){
			if(i == tab){
				((uiEntity*)n->data)->display = 1;
			}else{
				((uiEntity*)n->data)->display = 0;
			}
			i++;
			n = n->next;
		}
		ted->display_index = tab;
	}
	return 0;
}
uiEntity *uiTabEnvNew(const char *name){
	uiEntity *te = uiEntityNew(name,UI_ENT_TABENV);
	uiTabEnvData *ted = (uiTabEnvData*)malloc(sizeof(uiTabEnvData));
	ted->display_index = 0;
	te->data = ted;
	te->draw = uiTabEnvDraw;
	te->click = uiTabEnvClick;
	te->child = uiListNew();
	uiEntitySetMargin(te,UI_MARGIN_NORTH,UI_TAB_HEIGHT);
	return te;
}
int uiTabAdd(uiEntity *tab, uiEntity *tabenv){
	uiTabEnvData *ted = (uiTabEnvData*)tabenv->data;
	uiEntityAdd(tab,tabenv);
	if(uiTabCount(tabenv) == 1){
		ted->display_index = 0;
	}else{
		tab->display = 0;
	}
	if(!tab->namestring){
		tab->namestring = uiStringNew(tab->name,10,100,UI_TAB_HEIGHT,5,11);
	}
	return tabenv->child->count -1;
}

