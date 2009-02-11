#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include "hlTile.h"
#include "hlParam.h"
#include "hlDebug.h"
#define HL_NUM(p,x) p ->num[x]
#define HL_CLR(p,x) p ->color[x]

struct hl_param_prop_class{
	char *name;
	int  vector_size;
	int  order;
};
struct hl_param_class{
	char *name;
};

struct hl_param {
	int locked;
	int id; 
	unsigned int numc;
	float       *num;
	unsigned int colorc;
	hlColor     *color;
	hlImg       *img;
	hlState     state;
	hlCS 	cs;

	/*TODO : add vectors, bitmaps and gradients */
};


extern hlOpClass op_library[]; /*hlOp.c*/

hlParam *hlNewParam(enum hl_op_id id){
	hlParam* p = (hlParam*)malloc(sizeof(hlParam));
	hl_debug_mem_alloc(HL_MEM_PARAM);
	assert(op_library[id].id == (int)id);
	memset(p,0,sizeof(hlParam));
	p->id = id;
	
	p->numc = op_library[id].numc;
	p->num = (float*)malloc(p->numc*sizeof(float));
	hl_debug_mem_alloc_various(p->numc*sizeof(float));	
	memset(p->num,0,p->numc*sizeof(float));

	p->colorc = op_library[id].colorc;
	p->color = (hlColor*)malloc(p->colorc*sizeof(hlColor));
	hl_debug_mem_alloc_various(p->colorc*sizeof(hlColor));	
	memset(p->color,0,p->colorc*sizeof(hlColor));
	
	return p;
}
hlParam *hlDupParam(hlParam *p){
	hlParam *dup = (hlParam*)malloc(sizeof(hlParam));
	hl_debug_mem_alloc(HL_MEM_PARAM);
	memcpy((char*)dup,(char*)p,sizeof(hlParam));
	
	dup->num = (float*)malloc(p->numc*sizeof(float));
	hl_debug_mem_alloc_various(p->numc*sizeof(float));	
	memcpy((char*)dup->num,(char*)p->num,p->numc*sizeof(float));
	
	dup->color = (hlColor*)malloc(p->colorc*sizeof(hlColor));
	hl_debug_mem_alloc_various(p->colorc*sizeof(hlColor));	
	memcpy(	(char*)dup->color,
		(char*)p->color,
		p->colorc*sizeof(hlColor)	);
	return dup;
}
void hlFreeParam(hlParam *p){
	if(p->numc){ 
		free(p->num);
		hl_debug_mem_free_various(p->numc*sizeof(float));	
	}
	if(p->colorc){ 
		free(p->color);
		hl_debug_mem_free_various(p->colorc*sizeof(hlColor));	
	}
	free(p);
	hl_debug_mem_free(HL_MEM_PARAM);
}
int hlParamGetId(hlParam *p){
	return p->id;
}
void hlParamSetCS(hlParam *p, hlCS cs){
	assert(!p->locked);
	p->cs = cs;
}
hlCS hlParamGetCS(hlParam *p){
	return p->cs;
}
void hlParamSetNum(hlParam *p, ...){
	va_list ap;
	unsigned int i = 0;
	assert(!p->locked);
	assert(p->numc);
	va_start(ap,p);
	while(i < p->numc){
		p->num[i] = (float)(va_arg(ap, double));
		i++;
	}
	va_end(ap);
}
void hlParamSetVNum(hlParam *p,unsigned int numc, ...){
	va_list ap;
	unsigned int i = 0;
	assert(!p->locked);
	assert(p->numc >= numc);
	va_start(ap,numc);
	while(i < numc){
		p->num[i] = (float)(va_arg(ap, double));
		i++;
	}
	va_end(ap);
}
unsigned int hlParamGetNumCount(hlParam *p){
	return p->numc;
}
float *hlParamGetNum(hlParam *p){
	return p->num;
}
void hlParamSetColor(hlParam *p, ...){
	va_list cp;
	unsigned int i = 0;
	assert(!p->locked);
	assert(p->colorc);
	va_start(cp,p);
	while(i < p->colorc){
		p->color[i] = va_arg(cp, hlColor);
		i++;
	}
	p->cs = hlColorGetCS(p->color);
	va_end(cp);
}
hlColor* hlParamGetColor(hlParam *p){
	return p->color;
}
unsigned int hlParamGetColorCount(hlParam *p){
	return p->colorc;
}
void hlParamSetImg(hlParam *p,hlImg *img, hlState s){
	assert(!p->locked);
	p->img = img;
	p->state = s;
}
hlImg *hlParamGetImg(hlParam *p){
	return p->img;
}
hlState hlParamGetImgState(hlParam *p){
	return p->state;
}
void hlParamLock(hlParam *p){
	p->locked = 1;
}
int  hlParamIsLocked(hlParam *p){
	return p->locked;
}
void hlParamUnlock(hlParam *p){
	p->locked = 0;
}
void hlPrintParam(hlParam *p){
	int i = p->numc;
	printf("<hlParam>\n");
	printf("   id:%d\n",p->id);
	printf("   locked: ");
	if(p->locked)
		printf("true\n");
	else
		printf("false\n");
	printf("   numc:%d\n",i);
	while(i--){
		printf("   [%d]:%f\n",i,p->num[i]);
	}
	i = p->colorc;
	printf("   colorc:%d\n",i);
	while(i--){
		printf("   [%d]:",i);
		hlPrintColor(&(p->color[i]));
	}
	printf("   cs:\n");
	hlPrintCS(p->cs);
	printf("</hlParam>\n");
}

