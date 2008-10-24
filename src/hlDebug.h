#ifndef __HL_MEMORY_H__
#define __HL_MEMORY_H__

enum hl_mem_data{
	HL_MEM_TILE,
	HL_MEM_NODE,
	HL_MEM_FRAME,
	HL_MEM_PARAM,
	HL_MEM_OPERATION,
	HL_MEM_RAW,
	HL_MEM_IMG,
	HL_MEM_HASH,
	HL_MEM_UNKNOWN,
	HL_MEM_DATA_COUNT
};
void hl_debug_mem_alloc(enum hl_mem_data t);
void hl_debug_mem_alloc_various(int size);
void hl_debug_mem_free(enum hl_mem_data t);
void hl_debug_mem_free_various(int size);
void hl_print_mem_data(void);
#endif
