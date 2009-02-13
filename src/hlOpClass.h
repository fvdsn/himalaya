#ifndef __HL_OPERATION_CLASS__
#define __HL_OPERATION_CLASS__

#include "hlDataType.h"

enum hl_arg_type{
	HL_ARG_NUM,
	HL_ARG_COLOR,
	HL_ARG_IMG
};

int    	hlOpArgCount(const hlOp*op);
int	hlOpArgByName(const hlOp*op,const char *argname);
char* 	hlOpArgName(const hlOp*op, int arg);
char* 	hlOpArgDesc(const hlOp*op, int arg);
int	hlOpArgType(const hlOp*op, int arg);
int	hlOpArgSize(const hlOp*op, int arg);
float 	hlOpArgMinValue(const hlOp*op, int arg);
float 	hlOpArgMaxValue(const hlOp*op, int arg);

float   hlOpGetValue(const hlOp*op, int arg, int index);
hlColor hlOpGetColor(const hlOp*op, int arg, int index);
hlImg*	hlOpGetImg(const hlOp*op, int arg, int index);
hlState hlOpGetState(const hlOp*op, int arg, int index);

void	hlOpSetValue(const hlOp*op, int arg, int index, float value);
void	hlOpSetAllValue(const hlOp*op, int arg, int index, ... );
void	hlOpSetColor(const hlOp*op, int arg, int index, hlColor col);
void	hlOpSetImg(const hlOp*op, int arg, int index, hlImg *img, hlState s);

hlOpClass *hlNewOpClass(const char *name,
			const char *desc,
			int id, 
			int type, 
			void (*render)(hlTile *tile, hlParam *p) );

void hlOpClassAddNum( 	const char *name,
			const char *desc,
			int size,
			float min_value,
			float max_value		);

void hlOpClassAddColor( const char *name,
			const char *desc,
			int size 		);

void hlOpClassAddImage( const char *name,
			const char *desc,
			int size 		);

#endif

