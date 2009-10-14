#ifndef __HL_OPERATION_H__
#define __HL_OPERATION_H__

#include "hlDataType.h"
#include "hlColorSpace.h"
#include "hlColor.h"
#include "hlTile.h"
#include "hlFrame.h"

enum hl_op_id{
	HL_ADJ_SKIP,
	HL_ADJ_INVERT,
	HL_ADJ_FILL,
	HL_ADJ_TEST,
	HL_ADJ_ADD,
	HL_ADJ_MUL,
	HL_ADJ_DIV,
	HL_ADJ_MOD,
	HL_ADJ_LESS,
	HL_ADJ_MORE,
	HL_ADJ_POW,
	HL_ADJ_CHANMIX,
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

struct hl_op{
	struct hl_op* down;
	int locked;
	int caching;	/*always cache if true */
	int refcount; 	/*count of saved state depending on this op*/
	hlOpRef ref;	/*number unique to this operation, shared
			 *with operations duplicated from this one,
			 *because of modification of this op, or 
			 *a child one */
	int id;		/*what the operation does*/
	int type;	/*broad category of operation (blend,rot,...)*/
	void (*render)(hlTile *dst, hlOp *p);

	hlFrame *cache;
	hlImg 	*img;

	hlCS cs_in;
	hlCS cs_out;

	int p_numc;
	float *p_num;
	
	int p_colorc;
	hlColor *p_color;

	int p_imgc;
	hlImg **p_img;
	hlState*p_state;
};

/**
 * Create a new operation.
 * @param id 	: the kind of operation (HL_DRAW_RECT, ...)
 * @return	: a new operation, you may want to set it's
 * 	parameters with the operation arguments functions.
 */
hlOp* 	hlNewOp(int id);
/** 
 * Duplicates an operation.
 */
hlOp* 	hlDupOp(hlOp *p);
void 	hlFreeOp(hlOp* op);	/*TODO referenced states ... */
void 	hlPrintOp(hlOp *op);
void	hlOpLock(hlOp  *op);	/*TODO*/
void	hlOpUnlock(hlOp *op);
int	hlOpLocked(hlOp *op);

/*------------- OPERATION ARGUMENTS ---------- */

#define HL_MAX_ARG 8

enum hl_arg_type{
	HL_ARG_NUM = 0,	/* array of floats */
	HL_ARG_COLOR,	/* array of colors */
	HL_ARG_IMG	/* array of [hlImg,hlState] */
};

/**	
 * Returns the Count of arguments in an operation 
 * @param op 	: the operation.
 * @return	: the count [0,inf[ of arguments that this operation has.
 */
int    	hlOpArgCount(const hlOp*op);
/**
 * Returns the named argument's number/
 * @param op 	: the operation
 * @param argname	: the name of the argument.
 * @return	: -1 if no argument has that name or a number in
 * 		[0,Inf[ , the first argument having argname as a name.
 */
int	hlOpArgByName(const hlOp*op,const char *argname);
/**
 * Return the name of the argument
 * @param op : the operation.
 * @param arg : [0,hlOpArgCount[, the number of the argument.
 * @return : the name of the argument, or NULL if the argument doesn't exist.
 */
char* 	hlOpArgName(const hlOp*op, int arg);
/**
 * Return the description of the argument
 * @param op : the operation.
 * @param arg : [0,hlOpArgCount[, the number of the argument.
 * @return : the description of the argument, or NULL if the argument doesn't exist.
 */
char* 	hlOpArgDesc(const hlOp*op, int arg);
/**
 * Return the type of the argument
 * @param op : the operation.
 * @param arg : [0,hlOpArgCount[, the number of the argument.
 * @return : the type of the argument ( HL_ARG_NUM | HL_ARG_COLOR | HL_ARG_IMG
 * ) or HL_ARG_NUM if the argument doesn't exist.
 */
int	hlOpArgType(const hlOp*op, int arg);
/**
 * Return the number of values in the argument
 * @param op : the operation.
 * @param arg : [0,hlOpArgCount[, the number of the argument.
 * @return : the number of values in the argument, 0 if the argument doesn't 
 * exist
 */
int	hlOpArgSize(const hlOp*op, int arg);
/**
 * Return the advised minimum value that a numerical argument can have.
 * @param op : the operation.
 * @param arg : [0,hlOpArgCount[, the number of the argument.
 * @return : the minimum value that the numerical argument can have.
 * If the argument is not of type HL_ARG_NUM or doesn't exists : returns 0.0
 */
float 	hlOpArgMinValue(const hlOp*op, int arg);
/**
 * Return the advised maximum value that a numerical argument can have.
 * @param op : the operation.
 * @param arg : [0,hlOpArgCount[, the number of the argument.
 * @return : the minimum value that the numerical argument can have.
 * If the argument is not of type HL_ARG_NUM or doesn't exists : returns 0.0
 */
float 	hlOpArgMaxValue(const hlOp*op, int arg);
/**
 * Returns the current value of an operation argument.
 * @param op 	: the operation.
 * @param arg	: in [0,hlOpArgCount(op)[ the argument number.
 * @param index : in [0,hlOpArgSize(op,arg)[ the index in the value array.
 * @return 	: the value or 0.0 if the arg or index ar out of bounds or the
 * argument is not a value array.
 */
float   hlOpGetValue(const hlOp*op, int arg, int index);
float*	hlOpGetAllValue(const hlOp *op);
/**
 * Returns the color of an operation argument.
 * @param op 	: the operation.
 * @param arg	: in [0,hlOpArgCount(op)[ the argument number.
 * @param index : in [0,hlOpArgSize(op,arg)[ the index in the value array.
 * @return 	: the value or 0.0 if the arg or index ar out of bounds, or the
 * argument is not a color array.
 */
hlColor hlOpGetColor(const hlOp*op, int arg, int index);
hlColor*hlOpGetAllColor(const hlOp *op);
/**
 * Returns the image of an operation argument.
 * @param op 	: the operation.
 * @param arg	: in [0,hlOpArgCount(op)[ the argument number.
 * @param index : in [0,hlOpArgSize(op,arg)[ the index in the value array.
 * @return 	: the image or NULL if the arg or index ar out of bounds, or the
 * argument is not a image array.
 */
hlImg*	hlOpGetImg(const hlOp*op, int arg, int index);
hlImg** hlOpGetAllImg(const hlOp*op);
/**
 * Returns the state of an operation argument.
 * @param op 	: the operation.
 * @param arg	: in [0,hlOpArgCount(op)[ the argument number.
 * @param index : in [0,hlOpArgSize(op,arg)[ the index in the value array.
 * @return 	: the state or NULL if the arg or index ar out of bounds, or the
 * argument is not a image array.
 */
hlState hlOpGetState(const hlOp*op, int arg, int index);
hlState*hlOpGetAllState(const hlOp*op);
hlCS 	hlOpGetCSIn(const hlOp*op);
hlCS 	hlOpGetCSOut(const hlOp*op);
int	hlOpGetId(const hlOp*op);

void	hlOpSetValue(hlOp*op, int arg, int index, float value);
void	hlOpSetAllValue(hlOp*op, const char *argname, ... );
void	hlOpSetColor(hlOp*op, int arg, int index, hlColor col);
void	hlOpSetAllColor(hlOp*op, const char *argname, ... );
void	hlOpSetImg(hlOp*op, int arg, int index, hlImg *img, hlState s);
void	hlOpSetAllImg(hlOp*op, const char *argname, ... );
void	hlOpSetCSIn(hlOp*op, hlCS cs);
void	hlOpSetCSOut(hlOp*op, hlCS cs);

/*------------- OPERATION CLASS -------------*/

hlOpClass *hlNewOpClass(const char *name,
			const char *desc,
			int id, 
			int type, 
			void (*render)(hlTile *tile, hlOp *p) );

void hlOpClassAddNum( 	hlOpClass *c,
			const char *name,
			const char *desc,
			int size,
			float min_value,
			float max_value		);

void hlOpClassAddColor( hlOpClass *c,
			const char *name,
			const char *desc,
			int size 		);

void hlOpClassAddImage( hlOpClass *c,
			const char *name,
			const char *desc,
			int size 		);

/*------------- OPERATION CACHE -------------*/

void	hlOpCacheEnable(hlOp*op, int enabled);
/*removes every tile in the cache and frees it*/
void 	hlOpCacheFree(hlOp*op);
/*removes the tiles from the cache and returns it*/
hlTile *hlOpCacheRemove(hlOp* op, int x, int y, unsigned int z);

/*places a copy of the tile in the cache. frees the original one,
 * if it exists. tile should not be NULL. WARNING : any modification of
 * the tile after it has been set will modify the tile in the cache 
 * as well, use hlTileDup() */
void 	hlOpCacheSet(hlOp* op, hlTile*tile, hlCS cs, int sx, int sy, int tx, int ty,unsigned int tz);

/*returns the tile in the cache. returns NULL if there is no tile in
 * the cache. WARNING any modification of the returned tile will modify
 * the cache as well. use hlTileDup() if you plan to modify the tile*/
hlTile *hlOpCacheGet(hlOp* op, int tx, int ty, unsigned int tz);

#endif
