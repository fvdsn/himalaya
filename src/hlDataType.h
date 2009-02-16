#ifndef __HL_DATA_TYPE_H__
#define __HL_DATA_TYPE_H__

#include "hlTile.h"
typedef int hlState;
typedef int hlOpRef;
typedef struct hl_img hlImg;
typedef struct hl_param hlParam;
typedef struct hl_op hlOp;
typedef struct hl_op_class hlOpClass;
typedef struct hl_arg hlArg;

struct hl_op_class{
	int id;
	int type;
	char *name;
	char *desc;

	int numc;
	int colorc;
	int imgc;

	int argc;
	hlArg * arg;

	void (*render)(hlTile *tile, hlParam *p);
};

struct hl_arg{
	int type;
	char *name;
	char *desc;
	int index;
	int size;
	float min_value;
	float max_value;
};

#endif
