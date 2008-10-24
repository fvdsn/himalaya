#ifndef __HL_DATA_TYPE_H__
#define __HL_DATA_TYPE_H__

#include "hlTile.h"
typedef int hlState;
typedef struct hl_img hlImg;
typedef struct hl_param hlParam;
typedef struct hl_op hlOp;
typedef struct hl_op_class{
	unsigned int id;
	unsigned int type;
	unsigned int numc;
	unsigned int colorc;
	void (*render)(hlTile *tile, hlParam *p);
}hlOpClass;

#endif
