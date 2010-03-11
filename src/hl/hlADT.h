#ifndef __HL_ADT_H__
#define __HL_ADT_H__

typedef struct hl_hash_node{
	struct hl_hash_node * next;
	int key;
	void * data;
}hlHNode;

typedef struct hl_hashtable{
	int size;
	int objcount;
	hlHNode ** table;
}hlHash;

hlHash *hlNewHash(int size);
void 	hlFreeHash(hlHash *h);
void 	hlPrintHash(hlHash *h);

/* adds data to the hashtable with key1 = key, key2 = 0
 * in case of conflict, data is not added, and returns 1 */
int hlHashAdd(	hlHash *h, 
		int key, 
		void *data);
/* returns data from hashtable matching key, key2 
 * NULL if no match */
void *hlHashGet(hlHash *h, int key);
/* removes from hash every entry matching key1 == key, doesn't free
 * objects of course */
void *hlHashRem(hlHash *h, int key);
int hlHashCount(hlHash *h);

typedef struct hl_list_node{
	struct hl_list_node *next;
	void *data;
}hlLNode;

typedef struct hl_list{
	int size;
	hlLNode *first;
	hlLNode *last;
}hlList;

hlList *hlNewList(void);
void	hlListAppend(hlList *l, void *data);
void	hlListPush(hlList *l, void *data);
void *	hlListPop(hlList *l);
int	hlListRemData(hlList *l, void *data);
void	hlListRemAllData(hlList *l, void *data);
void *  hlListGet(const hlList *l, int index);
void *  hlListRem(hlList *l, int index);


#endif

