#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include "hlTile.h"
#include "hlParam.h"
#define HL_NUM(p,x) p ->num[x]
#define HL_CLR(p,x) p ->color[x]

struct hl_param {
	int 	locked;
	int 	id; 
	hlCS	cs;
	
	int 	numc;
	float*	num;
	
	int 	colorc;
	hlColor* color;
	
	int 	imgc;
	hlImg**	img;
	hlState* state;
	
};


extern hlOpClass op_library[]; /*hlOp.c*/
hlParam *hlNewParam(enum hl_op_id id){
	hlParam* p = (hlParam*)malloc(sizeof(hlParam));
	memset(p,0,sizeof(hlParam));
	p->locked = 0;
	p->id = id;
	
	p->numc = op_library[id].numc;
	p->num  = (float*)malloc(p->numc*sizeof(float));
	memset(p->num,0,p->numc*sizeof(float));

	p->colorc = op_library[id].colorc;
	p->color = (hlColor*)malloc(p->colorc*sizeof(hlColor));
	memset(p->color,0,p->colorc*sizeof(hlColor));

	p->imgc = op_library[id].imgc;
	p->img = (hlImg**)malloc(p->imgc*sizeof(hlImg*));
	memset(p->img,0,p->imgc*sizeof(hlImg*));

	p->state = (hlState*)malloc(p->imgc*sizeof(hlState));
	memset(p->state,0,p->imgc*sizeof(hlState));

	return p;
}
hlParam *hlDupParam(hlParam *p){
	hlParam *dup = hlNewParam(p->id);
	memcpy((char*)dup->num,	  (char*)p->num,   p->numc*sizeof(float));
	memcpy((char*)dup->color, (char*)p->color, p->colorc*sizeof(hlColor));
	memcpy((char*)dup->img,   (char*)p->img,   p->imgc*sizeof(hlImg*));
	memcpy((char*)dup->state, (char*)p->state, p->imgc*sizeof(hlState));
	return dup;
}
void hlFreeParam(hlParam *p){
	if(p->numc){ 
		free(p->num);
	}
	if(p->colorc){ 
		free(p->color);
	}
	if(p->imgc){
		free(p->img);
		free(p->state);
	}
	free(p);
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
void hlParamSetNum(hlParam *p, int index, float val){
	assert(!p->locked);
	p->num[index] = val;
}
int hlParamGetNumCount(hlParam *p){
	return p->numc;
}
float *hlParamGetNum(hlParam *p){
	return p->num;
}
void hlParamSetColor(hlParam *p, int index, hlColor col){
	assert(!p->locked);
	p->color[index] = col;
}
hlColor* hlParamGetColor(hlParam *p){
	return p->color;
}
int hlParamGetColorCount(hlParam *p){
	return p->colorc;
}
void hlParamSetImg(hlParam *p, int index, hlImg *img, hlState s){
	assert(!p->locked);
	p->img[index] = img;
	p->state[index] = s;
}
int	hlParamGetImgCount(hlParam *p){
	return p->imgc;
}
hlImg** hlParamGetImg(hlParam *p){
	return p->img;
}
hlState* hlParamGetState(hlParam *p){
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

