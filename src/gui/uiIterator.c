#include <stdlib.h>
#include <string.h>
#include "uiIterator.h"

static uiNode *uiNewNode(void *data){
	uiNode *n = (uiNode*)malloc(sizeof(uiNode));
	n->next = NULL;
	n->data = data;
	return n;
}
uiList *uiListNew(){
	uiList *l = (uiList*)malloc(sizeof(uiList));
	memset(l,0,sizeof(uiList));
	return l;
}
void uiListAdd(uiList *l, void *data){
	uiNode *n = uiNewNode(data);
	if(l->last){
		l->last->next = n;
		l->last = n;
	}else{
		l->first = n;
		l->last  = n;
	}
	l->count++;
}
void uiListRemove(uiList *l, void *data){
	uiNode *n = l->first;
	uiNode *prev = NULL;
	uiNode *next = NULL;
	if(!n){
		return;
	}
	while(n){
		next = n->next;
		if(n->data == data){
			if(!prev){
				l->first = next;
			}else{
				prev->next = next;
			}
			if(!next){
				l->last = prev;
			}
			l->count--;
			free(n);
		}
		n = next;
	}
}

