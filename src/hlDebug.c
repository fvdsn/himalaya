#include <stdio.h>
#include "hlDebug.h"
int refcount[HL_MEM_DATA_COUNT];
void hl_debug_mem_alloc(enum hl_mem_data i){
	refcount[i]++;
}
void hl_debug_mem_free(enum hl_mem_data i){
	refcount[i]--;
}
void hl_debug_mem_alloc_various(int size){
	refcount[HL_MEM_UNKNOWN]+=size;
}
void hl_debug_mem_free_various(int size){
	refcount[HL_MEM_UNKNOWN]-=size;
}
void hl_print_mem_data(){
	printf("MEMORY USAGE ===============\n");
	printf("\ttiles :%d\n",refcount[HL_MEM_TILE]);
	printf("\tnodes :%d\n",refcount[HL_MEM_NODE]);
	printf("\tframes :%d\n",refcount[HL_MEM_FRAME]);
	printf("\tparam :%d\n",refcount[HL_MEM_PARAM]);
	printf("\traws :%d\n",refcount[HL_MEM_RAW]);
	printf("\toperations :%d\n",refcount[HL_MEM_OPERATION]);
	printf("\timgs :%d\n",refcount[HL_MEM_IMG]);
	printf("\thashtable :%d\n",refcount[HL_MEM_HASH]);
	printf("\tunknown:%fMB\n",(float)refcount[HL_MEM_UNKNOWN]/1048576.0);
	return;
}
