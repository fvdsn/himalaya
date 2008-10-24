#ifndef __HL_PARAM_H__
#define __HL_PARAM_H__

#include "hlColorSpace.h"
#include "hlColor.h"
#include "hlDataType.h"

#define HL_OPAQUE    -1
#define HL_INVISIBLE  1
#define HL_SEMITRANS  0

enum hl_op_id{
	HL_ADJ_SKIP 	= 0,
	HL_ADJ_INVERT	= 1,
	HL_ADJ_FILL 	= 2,
	HL_ADJ_TEST 	= 3,
	HL_ADJ_ADD 	= 4,
	HL_ADJ_MUL 	= 5,
	HL_ADJ_DIV 	= 6,
	HL_ADJ_MOD 	= 7,
	HL_ADJ_LESS	= 8,
	HL_ADJ_MORE	= 9,
	HL_ADJ_POW      = 10,
	HL_ADJ_CHANMIX  = 11,
	HL_BLEND_MIX,
	HL_BLEND_ADD,
	HL_BLEND_MULT,
	HL_DRAW_RECT,
	HL_DRAW_CIRCLE,
	HL_OP_COUNT
};
enum hl_op_type{
	HL_ADJUSTMENT,
	HL_BLENDING,
	HL_DRAW,
	HL_TRANSFORM,
	HL_FILTER
};
hlParam* hlNewParam(enum hl_op_id id);
hlParam* hlDupParam(hlParam *p);
void 	hlFreeParam(hlParam *p);
void 	hlPrintParam(hlParam *p);
int	hlParamIsLocked(hlParam*p);
void 	hlParamLock(hlParam* p);
void 	hlParamUnlock(hlParam* p);

int	hlParamGetId(hlParam *p);	
void 	hlParamSetCS(hlParam *p, hlCS cs);
hlCS 	hlParamGetCS(hlParam *p);

void 	hlParamSetNum(hlParam *p, ...);    
void    hlParamSetVNum(hlParam *p,unsigned int numc, ...);
float * hlParamGetNum(hlParam *p);	
unsigned int 	hlParamGetNumCount(hlParam *p);


void 	hlParamSetColor(hlParam *p, ...); 
hlColor*hlParamGetColor(hlParam *p);	
unsigned int	hlParamGetColorCount(hlParam *p);

void 	hlParamSetImg(hlParam *p, hlImg *img, hlState s);
hlImg*  hlParamGetImg(hlParam *p);
hlState hlParamGetImgState(hlParam *p);	
#endif

