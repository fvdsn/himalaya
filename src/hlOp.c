#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "hlOp.h"

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

	void (*render)(hlTile *tile, hlOp *p);
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

hlOpClass op_library[HL_OP_COUNT];

/* 	hlOp_(...)	*/
static hlOpRef hl_new_ref(void){
	static hlOpRef ref = 0;
	return ref++;
}
hlOp* hlNewOp(int id){
	hlOp* op = (hlOp*)malloc(sizeof(hlOp));
	op->down 	= NULL;
	op->id 	 	= id;
	op->ref 	= hl_new_ref();
	op->refcount 	= 0;
	op->caching 	= 0;
	op->cache 	= NULL;
	assert(op_library[id].id == id);
	op->render 	= op_library[id].render;
	op->type 	= op_library[id].type;
	op->p_numc 	= op_library[id].numc;
	op->p_colorc	= op_library[id].colorc;
	op->p_imgc	= op_library[id].imgc;
	if(op->p_numc){
		op->p_num = (float*)malloc(op->p_numc*sizeof(float));
		memset(op->p_num,0,op->p_numc*sizeof(float));
	}
	if(op->p_colorc){
		op->p_color = (hlColor*)malloc(op->p_colorc*sizeof(hlColor));
		memset(op->p_color,0,op->p_colorc*sizeof(hlColor));
	}
	if(op->p_imgc){
		op->p_img = (hlImg**)malloc(op->p_imgc*sizeof(hlImg*));
		memset(op->p_img,0,op->p_imgc*sizeof(hlImg*));
		op->p_state = (hlState*)malloc(op->p_imgc*sizeof(hlState));
		memset(op->p_state,0,op->p_imgc*sizeof(hlState));
	}
	return op;
	/* TODO : CHECK THIS LATER :
	 * when creating a blending op that links against another image
	 * and state, we must duplicate the state, so that if the linked
	 * state is removed elsewhere, we still have a valid reference */ 
	return op;
}
hlOp* hlDupOp(hlOp* op){
	hlOp* dup = hlNewOp(op->id);
	float* num   = dup->p_num;
	hlColor* col = dup->p_color;
	hlImg **img  = dup->p_img;
	hlState *s   = dup->p_state;
	memcpy(dup,op,sizeof(hlOp));
	dup->p_num      = num;
	dup->p_color    = col;
	dup->p_img      = img;
	dup->p_state    = s;
	dup->refcount 	= 0;
	dup->cache 	= NULL;
	dup->caching 	= 0;
	if(op->p_numc){
		memcpy(dup->p_num,op->p_num,op->p_numc*sizeof(float));
	}
	if(op->p_colorc){
		memcpy(dup->p_color,op->p_color,op->p_colorc*sizeof(hlColor));
	}
	if(op->p_imgc){
		memcpy(dup->p_img,op->p_img,op->p_imgc*sizeof(hlImg*));
		memcpy(dup->p_state,op->p_state,op->p_imgc*sizeof(hlState));
	}
	return dup;
}
void hlFreeOp(hlOp* op){
	if(op->p_numc){
		free(op->p_num);
	}
	if(op->p_colorc){
		free(op->p_color);
	}
	if(op->p_imgc){
		free(op->p_img);
		free(op->p_state);
	}
	if(op->cache){hlFreeFrame(op->cache);}
	free(op);
}
void hlPrintOp(hlOp *op){
	int i = 0;
	printf("<Op> \n   id:%d\n   type:%d\n",op->id,op->type);
	if(op->caching){
		printf("   caching: true\n");
	}
	else{
		printf("   caching: false\n");
	}
	if(op->locked){
		printf("   locked:  true\n");
	}else{
		printf("   locked:  false\n");
	}
	printf("   down:%p\n",(void*)op->down);
	printf("   img:%p\n",(void*)op->img);
	printf("   refcount:%d\n",op->refcount);
	printf("   ref:%d\n",op->ref);
	printf("   param:\n");
	printf("   cache:\n");
	if(op->cache){
		hlPrintFrame(op->cache);
	}
	else{
		printf("NULL\n");
	}
	printf("   csin:\n");
	hlPrintCS(op->cs_in);
	printf("   p_numc:%d\n",op->p_numc);
	i = 0;
	while(i < op->p_numc){
		printf("        [%d] : %f \n",i,op->p_num[i]);
		i++;
	}
	printf("   p_colorc:%d\n",op->p_colorc);
	i = 0;
	while(i < op->p_colorc){
		hlPrintColor(&(op->p_color[i]));
		i++;
	}
	printf("</Op>\n");
	return;
}
void hlOpLock(hlOp *op){
	op->locked = 1;
}
void hlOpUnlock(hlOp *op){
	op->locked = 0;
}
int hlOpLocked(hlOp *op){
	return op->locked;
}

/* 	hlOpCache___(...) 		*/
void hlOpCacheEnable(hlOp *op, int enabled){
	op->caching = enabled;
}
void hlOpCacheFree(hlOp*op){
	if(op->cache)
		hlFreeFrame(op->cache);
}
hlTile *hlOpCacheRemove(hlOp* op, int x, int y, unsigned int z){
	if(op->cache){
		return hlFrameTileRemove(op->cache,x,y,z);
	}else{
		return NULL;
	}
}
void hlOpCacheSet(hlOp* op, hlTile*tile, hlCS cs, int sx, int sy, int tx, int ty, unsigned int tz){
	if(!op->cache){
		op->cache = hlNewFrame(	hlNewColor(cs,0,0,0,0,0),sx,sy);
	}
	hlFrameTileSet(op->cache,tile,tx,ty,tz);
	return;
}
hlTile *hlOpCacheGet(hlOp* op, int tx, int ty, unsigned int tz){
	if(op->cache){
		return hlFrameTileGet(op->cache,tx,ty,tz);
	}
	else{
		return NULL;
	}
}

/*------------- OPERATION ARGUMENTS ---------- */

static hlOpClass * hl_op_get_class(const hlOp*op){
	return op_library + op->id;
}
static hlArg* hl_op_get_arg(const hlOp*op, int arg){
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
	int arg = hlOpArgCount(op);
	while(arg--){
		if(!strcmp(hlOpArgName(op,arg),argname)){
			return arg;
		}
	}
	return -1;
}

int	hlOpGetId(const hlOp *op){
	return op->id;
}
float hlOpGetValue(const hlOp*op, int arg, int index){
	if(hlOpArgType(op,arg) == HL_ARG_NUM){
		if(index < 0 || index >= hlOpArgSize(op,arg)){
			printf("ERROR : hlOpGetValue(...) index out of bounds \n");
			index = 0;
		}
		return op->p_num[hl_op_get_arg(op,arg)->index];
	}else{
		printf("ERROR: hlOpGetValue(...) : arg is not numerical, returning 0.0\n");
		return 0.0;
	}
}
float *hlOpGetAllValue(const hlOp *op){
	return op->p_num;
}
hlColor hlOpGetColor(const hlOp*op, int arg, int index){
	if(hlOpArgType(op,arg) == HL_ARG_COLOR){
		if(index < 0 || index >= hlOpArgSize(op,arg)){
			printf("WARNING : hlOpArgColor(...) index out of bounds \n");
			index = 0;
		}
		return op->p_color[hl_op_get_arg(op,arg)->index];
	}else{
		printf("ERROR: hlOpArgColor(...) : arg is not a color, returning 8bit black\n");
		return hlNewColor(hlNewCS(HL_8B,HL_RGB),0.0,0.0,0.0,0.0,255.0);
	}
}
hlColor *hlOpGetAllColor(const hlOp *op){
	return op->p_color;
}
hlImg*	hlOpGetImg(const hlOp*op, int arg, int index){
	if(hlOpArgType(op,arg) == HL_ARG_IMG){
		if(index < 0 || index >= hlOpArgSize(op,arg)){
			printf("WARNING : hlOpGetImg(...) index out of bounds \n");
			index = 0;
		}
		return op->p_img[hl_op_get_arg(op,arg)->index];
	}else{
		printf("ERROR: hlOpGetImg(...) : arg is not an img, returning NULL\n");
		return NULL;	
	}
}
hlImg** hlOpGetAllImg(const hlOp *op){
	return op->p_img;
}
hlState hlOpGetState(const hlOp*op, int arg, int index){
	if(hlOpArgType(op,arg) == HL_ARG_IMG){
		if(index < 0 || index >= hlOpArgSize(op,arg)){
			printf("WARNING : hlOpGetState(...) index out of bounds \n");
			index = 0;
		}
		return op->p_state[hl_op_get_arg(op,arg)->index];
	}else{
		printf("ERROR: hlOpGetState(...) : arg is not an img, returning 0\n");
		return 0;	
	}
}
hlState *hlOpGetAllState(const hlOp *op){
	return op->p_state;
}
hlCS	hlOpGetCSIn(const hlOp *op){
	return op->cs_in;
}
hlCS	hlOpGetCSOut(const hlOp *op){
	return op->cs_out;
}
void	hlOpSetValue(hlOp*op, int arg, int index, float value){
	if(	hlOpArgType(op,arg) == HL_ARG_NUM 
		&& index >= 0 
		&& index < hlOpArgSize(op,arg)){
		op->p_num[ hl_op_get_arg(op,arg)->index + index] = value ;	
	}else{
		printf("ERROR : hlOpSetValue(...) arg is not numerical, value not set\n");
	}
}
void	hlOpSetAllValue(hlOp*op, const char *argname, ... ){
	va_list ap;
	int arg = hlOpArgByName(op,argname);
	int index = hlOpArgSize(op,arg);
	int i = 0;
	float f;
	va_start(ap,argname);
	while(i < index){
		f = (float)(va_arg(ap,double));
		hlOpSetValue(op,arg,i,f);
		i++;
	}
	va_end(ap);
}
void	hlOpSetColor(hlOp*op, int arg, int index, hlColor col){
	if(	hlOpArgType(op,arg) == HL_ARG_COLOR 
		&& index >= 0 
		&& index < hlOpArgSize(op,arg)){
		op->p_color[ hl_op_get_arg(op,arg)->index + index ] = col;
	}else{
		printf("ERROR : hlOpSetColor(...) arg is not a color, color not set\n");
	}
}
void	hlOpSetAllColor(hlOp*op, const char *argname, ... ){
	va_list ap;
	int arg = hlOpArgByName(op,argname);
	int index = hlOpArgSize(op,arg);
	int i = 0;
	va_start(ap,argname);
	while(i < index){
		hlOpSetColor(op,arg,i,va_arg(ap,hlColor));
		i++;
	}
	va_end(ap);
}
void	hlOpSetImg(hlOp*op, int arg, int index, hlImg *img, hlState s){
	if(	hlOpArgType(op,arg) == HL_ARG_IMG 
		&& index >= 0 
		&& index < hlOpArgSize(op,arg)){
		op->p_img[ hl_op_get_arg(op,arg)->index + index]   = img;
		op->p_state[ hl_op_get_arg(op,arg)->index + index] = s;
	}else{
		printf("ERROR : hlOpSetImg(...) arg is not an image, image not set\n");
	}
}
void	hlOpSetAllImg(hlOp*op, const char *argname, ... ){
	va_list ap;
	int arg = hlOpArgByName(op,argname);
	int index = hlOpArgSize(op,arg);
	int i = 0;
	va_start(ap,argname);
	while(i < index){
		hlOpSetImg(op,arg,i,va_arg(ap,hlImg*),va_arg(ap,hlState));
		i++;
	}
	va_end(ap);
}
void	hlOpSetCSIn(hlOp*op, hlCS cs){
	op->cs_in = cs;
}
void	hlOpSetCSOut(hlOp*op, hlCS cs){
	op->cs_out = cs;
}

/*------------- OPERATION CLASS ---------- */

hlOpClass *hlNewOpClass(const char *name,
			const char *desc,
			int id, 
			int type, 
			void (*render)(hlTile *tile, hlOp *p) ){
	hlOpClass * c = op_library + id;
	if(c->id){
		printf("ERROR : hlNewOpClass : redefining existing class : %d, was %d \n",id,c->id);
		return NULL;
	}
	c->name = (char*)name;	/*TODO : quite ugly */
	c->desc = (char*)desc;
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
	hlArg *a;
	a = c->arg + c->argc;
	a->name = (char*)name;
	a->desc = (char*)desc;
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
	hlArg *a;
	a = c->arg + c->argc;
	a->name = (char*)name;
	a->desc = (char*)desc;
	a->size = size;
	a->type = HL_ARG_COLOR;
	a->index = c->colorc;
	c->colorc += a->size;
	c->argc++;
	if(c->argc > HL_MAX_ARG){
		printf("ERROR hlOpClassAddColor(...) too much arguments to operation\n");
	}
}
void hlOpClassAddImage( hlOpClass *c,
			const char *name,
			const char *desc,
			int size		){
	hlArg *a;
	a = c->arg + c->argc;
	a->name = (char*)name;
	a->desc = (char*)desc;
	a->size = size;
	a->type = HL_ARG_IMG;
	a->index = c->imgc;
	c->imgc += a->size;
	c->argc++;
	if(c->argc > HL_MAX_ARG){
		printf("ERROR hlOpClassAddColor(...) too much arguments to operation\n");
	}
}
