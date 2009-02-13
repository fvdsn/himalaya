#include "hlOpClass.h"
#include "hlParam.h"

extern hlOpClass * op_library; /*hlOp.c*/

struct hl_op_class{
	int id;
	int type;
	char *name;
	char *desc;

	int numc;
	int colorc;
	int imgc;

	int argc;
	hlArg * arg

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

static hlOpClass * hl_op_get_class(hlOp*op){
	return op_library + op->id;
}
static hlArg* hl_op_get_arg(hlOp*op, int arg){
	hlOpClass *c = hl_op_get_class(op);
	if(arg < 0 || arg >= c->argc){
		printf("ERROR: hl_op_get_arg(...) arg %d out of bounds\n",arg);
		arg = 0;
	}
	return c->arg + arg;
}
int hlOpArgCount(const hlOp*op){
	return hl_op_get_class(op)->argc;
}
char* 	hlOpArgName(const hlOp*op, int arg){
	return hl_op_get_arg(op,arg)->name;
}
char* 	hlOpArgDesc(const hlOp*op, int arg){
	return hl_op_get_arg(op,arg)->desc;
}
int	hlOpArgType(const hlOp*op, int arg){
	return hl_op_get_arg(op,arg)->type;
}
int	hlOpArgSize(const hlOp*op, int arg){
	return hl_op_get_arg(op,arg)->size;
}
float 	hlOpArgMinValue(const hlOp*op, int arg){
	return hl_op_get_arg(op,arg)->min_value;
}
float 	hlOpArgMaxValue(const hlOp*op, int arg){
	return hl_op_get_arg(op,arg)->max_value;
}
int	hlOpArgByName(const hlOp*op,const char *argname){
	int argcount = hlOpArgCount(op);
	while(argcount--){
		if(!strcmp(hlOpArgName(op,arg),argname)){
			return argcount;
		}
	}
	return -1;
}

float hlOpGetValue(hlOp*op, int arg, int index){
	if(hlOpArgType(op,arg) == HL_ARG_NUM){
		if(index < 0 || index >= hlOpArgSize(op,arg)){
			printf("ERROR : hlOpGetValue(...) index out of bounds \n");
			index = 0;
		}
		return hlParamGetNum(op->param)[hl_op_get_arg(op,arg)->index];
	}else{
		printf("ERROR: hlOpGetValue(...) : arg is not numerical, returning 0.0\n");
		return 0.0;
	}
}
float hlOpGetColor(hlOp*op, int arg, int index){
	if(hlOpArgType(op,arg) == HL_ARG_COLOR){
		if(index < 0 || index >= hlOpArgSize(op,arg)){
			printf("WARNING : hlOpArgColor(...) index out of bounds \n");
			index = 0;
		}
		return hlParamGetColor(op->param)[hl_op_get_arg(op,arg)->index];
	}else{
		printf("ERROR: hlOpArgColor(...) : arg is not a color, returning 8bit black\n");
		return hlNewColor(hlNewCS(HL_8B,HL_RGB),0.0,0.0,0.0,0.0,255.0);
	}
}
hlImg*	hlOpGetImg(const hlOp*op, int arg, int index){
	return NULL	/* TODO */
}
hlState hlOpGetState(const hlOp*op, int arg, int index){
	return 0;	/* TODO */
}

void	hlOpSetValue(const hlOp*op, int arg, int index, float value){
	if(	hlOpArgType(op,arg) == HL_ARG_NUM 
		&& index >= 0 
		&& index < hlOpArgSize(op,arg)){
		hlParamSetNum(	op->param,
				hl_op_get_arg(op,arg)->index + index ,
				value	);	/*TODO */
	}else{
		printf("ERROR : hlOpSetValue(...) arg is not numerical, value not set\n");
	}
}
void	hlOpSetAllValue(const hlOp*op, int arg, int index, ... ){
}
void	hlOpSetColor(const hlOp*op, int arg, int index, hlColor col){
	if(	hlOpArgType(op,arg) == HL_ARG_COLOR 
		&& index >= 0 
		&& index < hlOpArgSize(op,arg)){
		hlParamSetColor( op->param,
				 hl_op_get_arg(op,arg)->index + index ,
				 col	);	/*TODO */
	}else{
		printf("ERROR : hlOpSetColor(...) arg is a color, color not set\n");
	}
}
void	hlOpSetImg(const hlOp*op, int arg, int index, hlImg *img, hlState s){
	if(	hlOpArgType(op,arg) == HL_ARG_IMG 
		&& index >= 0 
		&& index < hlOpArgSize(op,arg)){
		hlParamSetImg(	op->param,
				hl_op_get_arg(op,arg)->index + index ,
				img,
				state	);	/*TODO */
	}else{
		printf("ERROR : hlOpSetImg(...) arg is not an image, image not set\n");
	}
}
hlOpClass *hlNewOpClass(const char *name,
			const char *desc,
			int id, 
			int type, 
			void (*render)(hlTile *tile, hlParam *p) ){
	hlOpClass * c = op_library + id;
	if(c->id){
		printf("ERROR : hlNewOpClass : redefining existing class : %d, was %d \n",id,op->id);
		return;
	}
	c->name = name;
	c->desc = desc;
	c->id = id;
	c->type = type;
	c->numc = 0;
	c->colorc = 0;
	c->imgc = 0;
	c->render = render;
	c->argc = 0;
	c->arg = (hlArg*)malloc(HL_MAX_ARG *sizeof(hlArg));
	memset(c->arg,0,HL_MAX_ARG*sizeof(hlArg));
	return c;
}
void hlOpClassAddNum( 	hlOpClass *c,
			const char *name,
			const char *desc,
			int size,
			float min_value,
			float max_value		){
	hlArg *a
	a = c->arg + c->argc;
	a->name = name;
	a->desc = desc;
	a->size = size;
	a->min_value = min_value;
	a->max_value = max_value;
	a->type = HL_ARG_NUM;
	a->index = c->numc;
	c->numc += a->size;
	c->argc++;
	if(c->argc > HL_MAX_ARG){
		printf("ERROR hlOpClassAddNum(...) too much arguments to operation\n");
	}
}
void hlOpClassAddColor( hlOpClass *c,
			const char *name,
			const char *desc,
			int size		){
	hlArg *a
	a = c->arg + c->argc;
	a->name = name;
	a->desc = desc;
	a->size = size;
	a->type = HL_ARG_COLOR;
	a->index = c->numc;
	c->colorc += a->size;
	c->argc++;
	if(c->argc > HL_MAX_ARG){
		printf("ERROR hlOpClassAddColor(...) too much arguments to operation\n");
	}
}
void hlOpClassAddImage( const char *name,
			const char *desc,
			int size		){
	hlArg *a
	a = c->arg + c->argc;
	a->name = name;
	a->desc = desc;
	a->size = size;
	a->type = HL_ARG_IMG;
	a->index = c->numc;
	c->imgc += a->size;
	c->argc++;
	if(c->argc > HL_MAX_ARG){
		printf("ERROR hlOpClassAddColor(...) too much arguments to operation\n");
	}
}


