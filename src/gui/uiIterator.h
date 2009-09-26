#ifndef __UI_ITERATOR_H__
#define __UI_ITERATOR_H__

typedef struct ui_node{
	struct ui_node *next;
	void *data;
}uiNode;

typedef struct ui_linked_list{
	int count;
	uiNode *first;
	uiNode *last;
}uiList;



uiList *uiListNew(void);
void uiListAdd(uiList *list, void *data);
void uiListRemove(uiList *list, void *data);

#endif
