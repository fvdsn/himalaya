#include<stdlib.h>
#include<assert.h>
#include<stdio.h>
#include<string.h>
#include"hlADT.h"


static int hash(int size, int key){
	return key % size;
}
hlHash *hlNewHash(int size){
	hlHash *h = (hlHash*)malloc(sizeof(hlHash));
	assert(size);
	h->table = (hlHNode**)malloc(size * sizeof(hlHNode*));
	memset(h->table,0,size*sizeof(hlHNode*));
	h->size = size;
	h->objcount = 0;
	return h;
}
static hlHNode * hl_new_hnode(int key, void * data){
	hlHNode * hn = (hlHNode*)malloc(sizeof(hlHNode));
	hn->key = key;
	hn->data = data;
	hn->next = NULL;
	return hn;
}
void *hlHashGet(hlHash *h, int key){
	int index = hash(h->size,key);
	hlHNode *tmp = h->table[index];
	while(tmp){
		if(tmp->key == key ){
			return tmp->data;
		}
		tmp = tmp->next;
	}
	return NULL;
}
static int hl_add_node(hlHNode **dst, hlHNode *src){
	hlHNode *tmp;
	if ( *dst == NULL){
		*dst = src;
		return 0;
	}
	else {
		tmp = *dst;
		*dst = src;
		src->next = tmp;
		return 0;
	}
}
int hlHashAdd(hlHash *h, int key, void *data){
	assert(data);
	if (hlHashGet(h,key)){
		return 1;
	}
	else{
		hl_add_node( 	&(h->table[hash(h->size,key)]),
				hl_new_hnode(key,data)	);
		h->objcount++;
		return 0;
	}
}
void *hlHashRem(hlHash *h, int key){
	int index = hash(h->size,key);
	hlHNode *node = h->table[index];
	hlHNode **prev = &(h->table[index]);
	hlHNode *next = NULL;
	void * tmp = NULL;	
	while(node){
		next = node->next;
		if(node->key == key ){
			*prev = next;
			tmp = node->data;
			free(node);
			h->objcount--;
			return tmp;
		}
		prev = &node->next;
		node = node->next;
	}
	return NULL;
}
int hlHashCount(hlHash *h){
	return h->objcount;
}

/* --------- LIST ---------- */
hlList *hlNewList(void){
	hlList *list = (hlList*)malloc(sizeof(hlList));
	list->size = 0;
	list->first = NULL;
	list->last = NULL;
	return list;
}
static hlLNode * hl_new_list_node(void *data){
	hlLNode *ln = (hlLNode*)malloc(sizeof(hlLNode));
	ln->data = data;
	ln->next = NULL;
	return ln;
}
void hlListAppend(hlList *l, void *data){
	hlLNode *ln = hl_new_list_node(data);
	if(l->size == 0){
		l->first = ln;
		l->last = ln;
		l->size = 1;
	}else{
		l->last->next = ln;
		l->last = ln;
		l->size ++;
	}
}
void hlListPush(hlList *l, void *data){
	hlLNode *ln = hl_new_list_node(data);
	if(l->size == 0){
		l->first = ln;
		l->last = ln;
		l->size = 1;
	}else{
		ln->next = l->first;
		l->first = ln;
		l->size++;
	}
}
void *hlListPop(hlList *l){
	void *data = NULL;
	hlLNode *tmp = NULL;
	if(l->size == 0){
		return NULL;
	}else if(l->size == 1){
		data = l->first->data;
		free(l->first);
		l->first = NULL;
		l->last = NULL;
		l->size = 0;
		return data;
	}else{
		data = l->first->data;
		tmp = l->first->next;
		free(l->first);
		l->first = tmp;
		l->size--;
		return data;
	}
}
int hlListRemData(hlList *l, void *data){
	hlLNode *ln = l->first;
	hlLNode *prev = NULL;
	while(ln){
		if(ln->data == data){
			if(ln == l->first){
				l->first = ln->next;
			}
			if(ln == l->last){
				l->last = prev;
			}
			if(prev){
				prev->next = ln->next;
			}
			l->size--;
			free(ln);
			return 1;
		}
		prev = ln;
		ln = ln->next;
	}
	return 0;
}
void	hlListRemAllData(hlList *l, void *data){
	while(hlListRemData(l,data));
}
void *hlListGet(const hlList *l, int index){
	hlLNode *ln = l->first;
	if(index < 0 || index >= l->size){
		fprintf(stderr,"ERROR: hlListGet(%p,%d): index out of range [0,%d[\n",
			(void*)l,index,l->size);
		return NULL;
	}
	while(index-- && ln){
		ln = ln->next;
	}
	if(ln){
		return ln->data;
	}
	return NULL;
}
void *hlListRem(hlList *l, int index){
	hlLNode *ln = l->first;
	hlLNode *prev = NULL;
	void *data = NULL;
	if(index < 0 || index >= l->size){
		fprintf(stderr,"ERROR: hlListRem(%p,%d): index out of range [0,%d[\n",
			(void*)l,index,l->size);
		return NULL;
	}
	while(index-- && ln){
		prev = ln;
		ln = ln->next;
	}
	data = ln->data;
	if(!prev){
		l->first = ln->next;
	}else{
		prev->next = ln->next;
	}
	if(!ln->next){
		l->last = prev;
	}
	l->size--;
	free(ln);
	return data;
}
