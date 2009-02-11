#include<stdlib.h>
#include<assert.h>
#include<stdio.h>
#include"hlADT.h"
#include "hlDebug.h"


static unsigned int hash(unsigned int size, unsigned int key){
	return key % size;
}
hlHash *hlNewHash(unsigned int size){
	hlHash *h = (hlHash*)malloc(sizeof(hlHash));
	hl_debug_mem_alloc(HL_MEM_HASH);
	assert(size);
	h->table = (hlHNode**)malloc(size * sizeof(hlHNode*));
	hl_debug_mem_alloc_various(size*sizeof(hlHNode*));
	h->size = size;
	h->objcount = 0;
	return h;
}
hlHNode * hl_new_hnode(unsigned int key, void * data){
	hlHNode * hn = (hlHNode*)malloc(sizeof(hlHNode));
	hl_debug_mem_alloc_various(sizeof(hlHNode));
	hn->key = key;
	hn->data = data;
	hn->next = NULL;
	return hn;
}
void *hlHashGet(hlHash *h, unsigned int key){
	unsigned int index = hash(h->size,key);
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
int hlHashAdd(hlHash *h, unsigned int key, void *data){
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
void *hlHashRem(hlHash *h, unsigned int key){
	unsigned int index = hash(h->size,key);
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
			hl_debug_mem_free_various(sizeof(hlHNode));
			h->objcount--;
			return tmp;
		}
		prev = &node->next;
		node = node->next;
	}
	return NULL;
}
unsigned int hlHashCount(hlHash *h){
	return h->objcount;
}
/*
int main(int argc, char **argv){
	int d1 = 10;
	int d2 = 20;
	int d3 = 30;
	hlHash * h = hlNewHash(13);
	hlHashAdd(h,1,&d1);
	hlHashAdd(h,2,&d2);
	hlHashAdd(h,3,&d3);
	hlHashAdd(h,3,&d3);
	printf("d1:%d\n",*(int*)(hlHashGet(h,1)));
	printf("d2:%d\n",*(int*)(hlHashGet(h,2)));
	hlHashRem(h,3);
	printf("d3:%p\n",hlHashGet(h,3));
	return 0;
}*/
