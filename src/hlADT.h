#ifndef __HL_ADT_H__
#define __HL_ADT_H__

typedef struct hl_hash_node{
	struct hl_hash_node * next;
	unsigned int key;
	void * data;
}hlHNode;

typedef struct hl_hashtable{
	unsigned int size;
	unsigned int objcount;
	hlHNode ** table;
}hlHash;

hlHash *hlNewHash(unsigned int size);
void 	hlFreeHash(hlHash *h);
void 	hlPrintHash(hlHash *h);

/* adds data to the hashtable with key1 = key, key2 = 0
 * in case of conflict, data is not added, and returns 1 */
int hlHashAdd(	hlHash *h, 
		unsigned int key, 
		void *data);
/* returns data from hashtable matching key, key2 
 * NULL if no match */
void *hlHashGet(hlHash *h, unsigned int key);
/* removes from hash every entry matching key1 == key, doesn't free
 * objects of course */
void *hlHashRem(hlHash *h, unsigned int key);
unsigned int hlHashCount(hlHash *h);
#endif

