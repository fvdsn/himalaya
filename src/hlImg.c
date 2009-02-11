#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "hlImg.h"
#include "hlAdjOp.h"
#include "hlBlendOp.h"
#include "hlDrawOp.h"
#include "hlDebug.h"

struct hl_img{
	struct hl_op* top; 	/* last operation */
	hlState state;	/* unsaved, or the saved state id */
	hlRegion region;	
	hlHash * statelib; 	/* to quickly find state top op */
	hlFrame * source; 	/* original image data*/
};
struct hl_op{
	struct hl_op* down;
	int caching;	/*always cache if true */
	int refcount; 	/*count of saved state depending on this op*/
	hlOpRef ref;	/*number unique to this operation, shared
			 *with operations duplicated from this one,
			 *because of modification of this op, or 
			 *a child one */
	int id;		/*what the operation does*/
	int type;	/*broad category of operation (blend,rot,...)*/
	void (*render)(hlTile *dst, hlParam *p);
	struct hl_param *param;	
	hlFrame *cache;
	hlImg 	*img;
};

hlOpClass op_library[HL_OP_COUNT] = { /*see hlParam.h*/
	/* column 0 : Operation ID
	 * column 1 : Operation Type
	 * column 2 : Count of float parameters
	 * column 3 : Count of color parameters
	 * column 4 : Op. render function
	 */
	{ HL_ADJ_SKIP,	 HL_ADJUSTMENT, 0, 0, hlAdjOpSkip},
	{ HL_ADJ_INVERT, HL_ADJUSTMENT, 0, 0, hlAdjOpInvert},
	{ HL_ADJ_FILL,	 HL_ADJUSTMENT, 0, 1, hlAdjOpFill},
	{ HL_ADJ_TEST,	 HL_ADJUSTMENT, 2, 2, NULL},
	{ HL_ADJ_ADD, 	 HL_ADJUSTMENT, 5, 0, hlAdjOpMath},
	{ HL_ADJ_MUL,    HL_ADJUSTMENT, 5, 0, hlAdjOpMath},
	{ HL_ADJ_DIV, 	 HL_ADJUSTMENT, 5, 0, hlAdjOpMath},
	{ HL_ADJ_MOD, 	 HL_ADJUSTMENT, 5, 0, hlAdjOpMath},
	{ HL_ADJ_LESS,	 HL_ADJUSTMENT, 5, 0, hlAdjOpMath},
	{ HL_ADJ_MORE,	 HL_ADJUSTMENT, 5, 0, hlAdjOpMath},
	{ HL_ADJ_POW,	 HL_ADJUSTMENT, 5, 0, hlAdjOpMath},
	{ HL_ADJ_CHANMIX,HL_ADJUSTMENT,25, 0, hlAdjOpChanmix},

	{ HL_BLEND_MIX,  HL_BLENDING,	2, 0, NULL}, /*hlBlendOp(...)*/
	{ HL_BLEND_ADD,  HL_BLENDING,   2, 0, NULL},
	{ HL_BLEND_MULT, HL_BLENDING,   2, 0, NULL},
	
	{ HL_DRAW_RECT,  HL_DRAW,       5, 1, NULL}, /*hlDrawOp(...)*/
	{ HL_DRAW_CIRCLE,HL_DRAW,       5, 1, NULL}
};

hlImg* hlNewImg(hlColor color,int sx, int sy){
	hlImg* img = (hlImg*)malloc(sizeof(hlImg));
	hl_debug_mem_alloc(HL_MEM_IMG);
	img->top 	= NULL;
	img->region 	= hlNewRegion(0,0,sx,sy,0);
	img->statelib 	= hlNewHash(1009);
	img->state 	= HL_STATE_UNSAVED;
	img->source 	= hlNewFrame(color,sx,sy);
	return img;
}
hlImg* hlNewImgFromSource(hlFrame *frame){
	hlImg* img = (hlImg*)malloc(sizeof(hlImg));
	hl_debug_mem_alloc(HL_MEM_IMG);
	img->top 	= NULL;
	img->region 	= hlNewRegion(	0,0,
					hlFrameSizeX(frame,0),
					hlFrameSizeY(frame,0),
					0 			);
	img->statelib 	= hlNewHash(1009);
	img->state 	= HL_STATE_UNSAVED;
	img->source 	= frame;
	hlFrameMipMap(frame);	/*TODO remove that requirement */ 
	return img;
}

/*------------- CLEANUP OPERATIONS -------------*/

/** 
 * empties the caches of all operations above and equal index.
 * this is used when you insert or remove an operation, the
 * cached results above are not valid anymore and must be 
 * purged 
 */
/*static void hl_img_purge_cache(hlImg *img, hlOp * op){
	hlOp* tmp = img->top;
	while(tmp){
		if(tmp->cache){
			hlFramePurge(op->cache);
		}
		if(tmp == op){
			return;
		}
		tmp = tmp->down;
	}
}*/

/*------------- STATE -------------*/

hlOp* hl_img_get_top_op(hlImg *img, hlState state){
	if(state != HL_STATE_UNSAVED && state != HL_STATE_CURRENT){
		return (hlOp*)hlHashGet(img->statelib,state);
	}
	else {
		return img->top;
	}
}
hlState hl_get_new_state(){
	static int state = 1;
	return state++;
}
hlState hlImgStateSave(hlImg *img){
	hlState state = hl_get_new_state();
	hlOp * op = img->top;
	hlHashAdd(img->statelib,(int)state,op);
	hlOpCacheEnable(op);
	while(op){
		op->refcount++;
		op = op->down;
	}
	img->state = state;
	return state;
}
hlState hlImgStateDup(hlImg *img,hlState s){
	hlState ns;
	hlOp *op;
	if(!hlImgStateExists(img,s)){
		printf("Cannot duplicate non existing state\n");
		return 0;
	}else{
		op = (hlOp*)hlHashGet(img->statelib,(int)s);
		ns = hl_get_new_state();
		hlHashAdd(img->statelib,(int)ns,op);
		while(op){
			op->refcount++;
			op = op->down;
		}
		return ns;
	}
}

hlState hlImgStateLoad(hlImg *img, hlState state){
	hlState oldstate = img->state;
	hlOp *op = img->top;
	hlOp *tmp = NULL;
	if (!hlImgStateExists(img,state)){
		printf("Cannot load non existing state\n");
		return 0;
	}else if (oldstate == state){
		return state;
	}else {
		if (oldstate == HL_STATE_UNSAVED){
			/* free unsaved op */
			while(op){
				tmp = op->down;
				if(op->refcount == 0){
					hlFreeOp(op);
				}
				else{
					break;
				}
				op = tmp;
			}
		}
		img->top = (hlOp*)hlHashGet(img->statelib,state);
		img->state = state;
		return oldstate;
	}
}

hlState hlImgStateGet(hlImg *img){
	return img->state;
}
hlState hlImgStateExists(hlImg *img, hlState state){
	if (hlHashGet(img->statelib,state)){
		return 1;
	}
	else{
		return 0;
	}
}
hlState hlImgStateRem(hlImg *img, hlState remstate){
	hlOp *op = NULL;
	hlOp *tmp = NULL;
	assert(remstate != img->state);
	assert(remstate != HL_STATE_UNSAVED);
	assert(hlImgStateExists(img,remstate));
	op = (hlOp*)hlHashGet(img->statelib,remstate);
	hlOpCacheDisable(op);
	while(op){
		tmp = op->down;
		op->refcount--;
		if(op->refcount == 0){
			hlFreeOp(op);
		}
		op = tmp;
	}
	hlHashRem(img->statelib,remstate);
	return 0;
}

/* 	hlImgSetRegion(...) 	*/
void hlImgSetRegion(hlImg *img, hlRegion r){
	img->region = r;
}
hlRegion hlImgGetRegion(hlImg *img){
	return img->region;
}

/*------------- OPERATION -------------*/

/* 	hlImgInsertOp(...) / hlImgRemoveOp(...) / etc... 	*/
int hlImgGetOpCount(hlImg *img, hlState state){
	hlOp* top = hl_img_get_top_op(img,state);
	int i = 0;
	while(top){
		top = top->down;
		i++;
	}
	return i;
}
/*static hlOp* hl_img_get_op(hlImg *img, hlState state, hlOpRef ref){
	hlOp *op = hl_img_get_top_op(img,state);
	while(op){
		if(op->ref == ref){
			return op;
		}else if(op->ref < ref){	/TODO this could blow things up/
			return NULL;
		}
		op = op->down;
	}
	return NULL;
}*/
static void insert_op(hlOp *up, hlOp* newop, hlOp *down){
	if(newop == NULL){
		printf("WARNING : insert_op: inserting NULL op, nothing inserted\n");
		return;
	}
	if(up == NULL){ /*inserting top op*/
		if(down == NULL){ /*inserting op in empty list*/
			newop->down = NULL;
		}else{
			newop->down = down;
		}
	}else{
		if(down == NULL){ /*inserting bottom op*/
			newop->down = NULL;
		}else{ /*inserting op in middle of op list */
			newop->down = down;
		}
		up->down = newop;
	}
	hlParamLock(newop->param);
	/*TODO check for forked states */
}
hlOpRef hlImgPopOp(hlImg *img){
	hlOp *tmp = NULL;
	hlOpRef ref = 0;
	img->state = HL_STATE_UNSAVED;
	if(img->top){
		ref = img->top->ref;
		tmp = img->top;
		img->top = tmp->down;
		if(tmp->refcount == 0){
			hlFreeOp(tmp);
		}
		return ref;
	}else{
		printf("WARNING: hlImgPopOp(...) : nothing to pop.\n");
		return 0;
	}
}
hlOpRef hlImgPushNewOp(hlImg *img, hlParam *p){
	hlOp* newop = hlNewOp(p);
	insert_op(NULL,newop,img->top);
	img->state  = HL_STATE_UNSAVED;
	img->top    = newop;
	newop->img = img;
	return newop->ref;
}
hlOpRef lImgPushOp(hlImg *img, hlOp* op){
	insert_op(NULL,op,img->top);
	img->state  = HL_STATE_UNSAVED;
	img->top    = op;
	op->img = img;
	return op->ref;
}
/**
 * if the current state is unsaved, and you want to modify a modop, you
 * need to clean the cache of the higher op if they are unsaved(refcount == 0)
 * else, you need to fork them just as with saved state 
 *
 * returns the operation with op->ref == ref. img->top is the forked one,
 * inconsistent caches are cleaned, and saved op are forked.
 */
static hlOp * hlImgForkOp(hlImg *img, hlOpRef ref){
	hlOp* op = img->top;
	hlOp* op_up = NULL;
	while(op && op->refcount == 0){
		hlOpCacheFree(op);
		if( op->ref == ref){ 
			return op;
		}
		op_up = op;
		op = op->down;
	}
	while(op){
		if(op == img->top){
			op = hlDupOp(op);
			img->top = op;
		}else{
			op = hlDupOp(op);
			op_up->down = op;
		}
		if(op->ref == ref){
			return op;
		}
		op_up = op;
		op = op->down;
	}
	printf("WARNING: hlImgForkOp(...): ref not found, has forked anyway\n");
	return NULL;
}
static hlOp *hl_find_op(hlImg *img, hlOpRef ref){ /*TODO make it faster than O(n)*/
	hlOp* op = img->top;
	while(op){
		if (op->ref == ref){
			return op;
		}
		op = op->down;
	}
	return NULL;
}
hlParam * hlImgModOpBegin(hlImg *img,hlOpRef ref){
	hlOp * op = hl_find_op(img,ref);
	if(!op){
		printf("ERROR: hlImgModOpBegin(...) operation not found.\n");
		return NULL;
	}else{
		op = hlImgForkOp(img,ref);
		hlParamUnlock(op->param);
		return op->param;
	}
}
void hlImgModOpEnd(hlImg *img, hlOpRef ref){
	hlOp *op = hl_find_op(img,ref);
	if(!op){
		printf("ERROR: hlImgModOpEnd(...) operation not found.\n");
	}else{
		hlParamLock(op->param);
	}
}

/* 	hlPrintImg(...) 	*/
void hlPrintImg(hlImg *img, hlState state){
	int i = hlImgGetOpCount(img,state);
	hlOp* tmp;
	printf("<hlImg>\n");
	if(img->state){
		printf("   current state:%d\n",img->state);
	}else{
		printf("   current state: UNSAVED\n");
	}
	if(state && state != img->state){
		printf("   printed state:%d\n",state);
	}else{
		printf("   printed state: CURRENT STATE\n");
	}
	printf("   opcount: %d\n   operation_list:\n",i);
	tmp = hl_img_get_top_op(img,state);
	while(i--){
		printf("   [%d] ->\n", i);
		hlPrintOp(tmp);
		tmp = tmp->down;
	}
	printf("   source: \n");
	hlPrintFrame(img->source);
	printf("</hlImg>\n");
	return;
}

/* 	hlImgSize_(...)	/ hlImgTile_(...)	*/
uint32_t hlImgSizeX(hlImg *img, unsigned int z){
	return hlFrameSizeX(img->source,z);
}
uint32_t hlImgSizeY(hlImg *img, unsigned int z){
	return hlFrameSizeY(img->source,z);
}
uint32_t hlImgTileX(hlImg *img, unsigned int z){
	return hlFrameTileX(img->source,z);
}
uint32_t hlImgTileY(hlImg *img, uint32_t z){
	return hlFrameTileY(img->source,z);
}
uint32_t hlImgDepth(hlImg *img){
	return hlFrameDepth(img->source);
}
hlCS	hlImgCS(hlImg *img){
	if(img->top){
		return hlParamGetCS(img->top->param);
	}
	else{
		return hlFrameCS(img->source); /*TODO : colorspace changes */
	}
}

/* 	hlImgTileRead(...) / hlImgTileCopy(...) */
hlTile *hlImgTileRead(hlImg *img, hlState state, int tx, int ty, unsigned int tz){
	hlOp *top = hl_img_get_top_op(img,state);
	if(top){ 
		assert(top->cache);
		return hlFrameTileRead(top->cache,tx,ty,tz);
	}
	else{
		return hlFrameTileRead(img->source,tx,ty,tz);
	}
}
hlTile *hlImgTileCopy(hlImg *img, hlState state, int x, int y, unsigned int z){
	return hlTileDup(hlImgTileRead(img,state,x,y,z),hlImgCS(img));
}
/***********************************************+
 * 	Operation functions 			|
 * *********************************************/

/* 	hlImgRender__(...) 		*/
void hlImgRenderTile(hlImg *img, hlState state, int x, int y, unsigned int z){
	hlOp *top = hl_img_get_top_op(img,state);
	if (top){
		hlOpRenderTile(top,true,x,y,z);
	}
}
void hlImgRenderRegion(hlImg *img, hlRegion r, hlState state){
	hlOp *top = hl_img_get_top_op(img,state);
	unsigned int x = r.tx;
	unsigned int y = r.ty;
	while(y--){
		x = r.tx;
		while(x--){
			if(top){
			hlOpRenderTile(	top,true,
					r.ptx + x,
					r.pty + y,
					r.z	);
			}
		}
	}
}

hlFrame * hlImgReadFrame(hlImg *img, hlState state){
	hlOp* top = hl_img_get_top_op(img,state);
	if(top){
		return top->cache;
	}
	else{
		return img->source;
	}
}
hlRaw *hlImgRenderNewRaw(hlImg *img, hlRegion r, hlState state){
	hlImgRenderRegion(img,r,state);
	return hlRawFromRegion(hlImgReadFrame(img,state),r);
}
void  hlImgRenderToRaw(hlImg *img, hlRaw *raw, hlState state, int px, int py, unsigned int z){
	hlImgRenderRegion(img, 
		hlNewRegion(px,py,hlRawSizeX(raw),hlRawSizeY(raw),z),state);
	hlRegionToRaw(hlImgReadFrame(img,state),raw,px,py,z);
}

/* 	OPERATIONS :	hl__Op(...) 	*/
static hlOpRef hl_new_ref(void){
	static hlOpRef ref = 0;
	return ref++;
}
hlOp* hlNewOp(hlParam *p){
	hlImg*img;
	hlState s;
	hlOp* op = (hlOp*)malloc(sizeof(hlOp));
	hl_debug_mem_alloc(HL_MEM_OPERATION);
	op->param 	= p;
	op->down 	= NULL;
	op->id 	 	= hlParamGetId(p);
	op->ref 	= hl_new_ref();
	op->refcount 	= 0;
	op->caching 	= 0;
	op->cache 	= NULL;
	assert(op_library[hlParamGetId(p)].id == op->id);
	op->render 	= op_library[hlParamGetId(p)].render;
	op->type 	= op_library[hlParamGetId(p)].type;
	/* TODO : THIS IS UGLY AS HELL.
	 * when creating a blending op that links against another image
	 * and state, we must duplicate the state, so that if the linked
	 * state is removed elsewhere, we still have a valid reference */
	if (op->type == HL_BLENDING){
		img = hlParamGetImg(p);
		assert(img && "blending op parameters need an image");
		if((s = hlParamGetImgState(p))!=0){
			hlParamSetImg(p,img,hlImgStateDup(img,s));
		}
	}
	return op;
}
hlOp* hlDupOp(hlOp* op){
	hlOp* dup = hlNewOp(hlDupParam(op->param));
	memcpy(dup,op,sizeof(hlOp));
	dup->refcount 	= 0;
	dup->cache 	= NULL;
	dup->caching 	= 0;
	return dup;
}
void hlFreeOp(hlOp* op){
	if(op->param){
		if(hlParamGetImg(op->param) && hlParamGetImgState(op->param)){
			hlImgStateRem(	hlParamGetImg(op->param),
					hlParamGetImgState(op->param));
		}
		hlFreeParam(op->param);
	}
	if(op->cache){hlFreeFrame(op->cache);}
	free(op);
	hl_debug_mem_free(HL_MEM_OPERATION);
}
void hlPrintOp(hlOp *op){
	printf("<Op> \n   id:%d\n   type:%d\n",op->id,op->type);
	if(op->caching){
		printf("   caching: true\n");
	}
	else{
		printf("   caching: false\n");
	}
	printf("   down:%p\n",(void*)op->down);
	printf("   img:%p\n",(void*)op->img);
	printf("   refcount:%d\n",op->refcount);
	printf("   ref:%d\n",op->ref);
	printf("   param:\n");
	hlPrintParam(op->param);
	printf("   cache:\n");
	if(op->cache){
		hlPrintFrame(op->cache);
	}
	else{
		printf("NULL\n");
	}
	printf("</Op>\n");
	return;
}

/* 	hlOpCache___(...) 		*/
void hlOpCacheEnable(hlOp *op){
	op->caching = 1;
}
void hlOpCacheDisable(hlOp *op){
	op->caching = 0;
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
void hlOpCacheSet(hlOp* op, hlTile*tile, int tx, int ty, unsigned int tz){
	if(!op->cache){
		op->cache = hlNewFrame(
				hlFrameColor(op->img->source),
				hlImgSizeX(op->img,0),
				hlImgSizeY(op->img,0)	);
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

/* 	hlOpRenderTile(...) 		*/
hlTile *hl_op_render_adj(hlOp* op, bool top, int x, int y, unsigned int z){
	const hlCS cs = hlParamGetCS(op->param);
	hlTile *tile = hlOpCacheGet(op,x,y,z);
	if(tile){ 			/*found in cache*/
		if(top){
			return tile;
		}
		else { 
			tile = hlTileDup(tile,cs);
			if (op->caching){
				return tile;
			}
			else{
				hlOpCacheRemove(op,x,y,z);
				return tile;
			}
		}
	}
	else{ 				/*tile not in cache*/
		if(op->down){
			tile = hlOpRenderTile(op->down,0,x,y,z);
		}
		else{
			tile = hlFrameTileCopy(op->img->source,x,y,z);
		}
		op->render(tile,op->param);
		if(op->caching || top ){ 	/*put tile in cache*/
			if(top){
				hlOpCacheSet(op,tile,x,y,z);
				return tile;
			}
			else{
				hlOpCacheSet(op,tile,x,y,z);
				return hlTileDup(tile,cs);
			}
		}
		else{
			return tile;
		}
	}
}
hlTile *hl_op_render_blend(hlOp* op, bool top, int x, int y, unsigned int z){
	const hlCS cs = hlParamGetCS(op->param);
	hlTile *tile = hlOpCacheGet(op,x,y,z);
	const hlTile *tileup = hlImgTileRead(	hlParamGetImg(op->param),
					hlParamGetImgState(op->param),
					x,y,z	);
	if(tile){ 			/*found in cache*/
		if(top){
			return tile;
		}
		else {	/* if not top we need to return a duplicate of the
			tile, because the tile we return will be modified
			further */
			tile = hlTileDup(tile,cs);
			if (op->caching){
				return tile;
			}
			else{
				hlOpCacheRemove(op,x,y,z);
				return tile;
			}
		}
	}
	else{ 				/*tile not in cache*/
		/* since the result of the operation was not cached,
		 * we need to render it, for that we need the tile result
		 * of the previous operation. we will overwrite the
		 * result of the current op on that tile and return it */
		if(op->down){
			tile = hlOpRenderTile(op->down,0,x,y,z);
		}
		else{
			tile = hlFrameTileCopy(op->img->source,x,y,z);
		}
		hlBlendOp(tile,tileup,op->param);
		if(op->caching || top ){ 	/*put tile in cache*/
			if(top){
				hlOpCacheSet(op,tile,x,y,z);
				return tile;
			}
			else{	/* if not top operation, the tile will
				be modified later, so we return a copy
				of the tile */
				hlOpCacheSet(op,tile,x,y,z);
				return hlTileDup(tile,cs);
			}
		}
		else{
			return tile;
		}
	}
}
hlTile *hl_op_render_draw(hlOp* op, bool top, int x, int y, unsigned int z){
	hlCS cs = hlParamGetCS(op->param);
	hlTile *tile = hlOpCacheGet(op,x,y,z);
	if(tile){ 			/*found in cache*/
		if(top){
			return tile;
		}
		else { 
			if (op->caching){
				return hlTileDup(tile,cs);
			}
			else{
				return hlOpCacheRemove(op,x,y,z);
			}
		}
	}
	else{ 				/*tile not in cache*/
		if(op->down){
			tile = hlOpRenderTile(op->down,0,x,y,z);
		}
		else{
			tile = hlFrameTileCopy(op->img->source,x,y,z);
		}
		hlDrawOp(tile,op->param,x,y,z);
		if(op->caching || top ){ 	/*put tile in cache*/
			if(top){
				hlOpCacheSet(op,tile,x,y,z);
				return tile;
			}
			else{
				hlOpCacheSet(op,tile,x,y,z);
				return hlTileDup(tile,cs);
			}
		}
		else{
			return tile;
		}
	}
}
hlTile *hlOpRenderTile(hlOp* op, bool istop, int x, int y, unsigned int z){
	switch (op->type){
		case HL_ADJUSTMENT:
			return hl_op_render_adj(op,istop,x,y,z);
		case HL_BLENDING:
			return hl_op_render_blend(op,istop,x,y,z);
		case HL_DRAW:
			return hl_op_render_draw(op,istop,x,y,z);
		default:
			assert(0 && "operation type not supported");
			return NULL;
	}
}

int main(int argc, char**argv){
 	hlCS cs	    = hlNewCS(HL_8B,HL_RGB);	/*colorspace*/
	hlRaw *in   = hlRawFromPng("girl_.png");
	hlImg *img  = hlNewImgFromSource(hlFrameFromRaw(in));
	hlColor c   = hlNewColor(cs,255.0,128.0,0.0,0.0,255.0);

	/*we create a new operation drawing a orange rectangle */
	hlParam *p = hlNewParam(HL_DRAW_RECT);
		hlParamSetColor(p,c);
		hlParamSetNum(p,100.0,120.0, 500.0,400.0, 0.8);
	hlOpRef r = hlImgPushNewOp(img,p);
	
	hlState s1 = hlImgStateSave(img);
	
	/*we create a new operation drawing a purple rectangle */
	p = hlNewParam(HL_DRAW_RECT);
		hlParamSetColor(p,hlNewColor(cs,255.0,0.0,128.0,0.0,255.0));
		hlParamSetNum(p,20.0,60.0, 200.0,300.0, 0.2);
	hlImgPushNewOp(img,p);

	/*we create a new operation drawing a green rectangle */
	p = hlNewParam(HL_DRAW_RECT);
		hlParamSetColor(p,hlNewColor(cs,20.0,255.0,0.0,0.0,255.0));
		hlParamSetNum(p,400.0,20.0, 500.0,300.0, 0.5);
	hlImgPushNewOp(img,p);

	hlState s2 = hlImgStateSave(img);


	hlRaw *out = hlNewRaw(cs,600,600);	/*an output buffer*/

	hlImgRenderToRaw(img, out, s1, 0, 0, 0);
	hlRawToPng( out, "blending-s1.png");

	
	hlImgRenderToRaw(img, out, s2, 0, 0, 0);
	hlRawToPng( out, "blending-s2.png");
	
	/* We modify the orange rectangle to be drawn in red */
	p = hlImgModOpBegin(img,r);
		hlParamSetColor(p,hlNewColor(cs,255.0,0.0,0.0,0.0,255.0));
	hlImgModOpEnd(img,r);

	/* we render our modifications zoomed out and centered*/
	hlImgRenderToRaw(img, out, HL_STATE_CURRENT, -200, -200, 2);
	hlRawToPng( out, "blending-s3.png");

	return 0;
}

	
/*int main(int argc, char** argv){
	hlCS cs     = hlNewCS(HL_8B,HL_RGB);
	hlRaw *in   = hlRawFromPng("girl_.png");
	hlRaw *in2  = hlRawFromPng("light2.png");
	hlImg *img  = hlNewImgFromSource(hlFrameFromRaw(in));
	hlImg *img2 = hlNewImgFromSource(hlFrameFromRaw(in2));
	hlColor c = hlNewColor(cs,0.0, 0.0, 0.0, 0.0, 255.0);
	hlParam *p3 = hlNewParam(HL_DRAW_RECT);
		hlParamSetColor(p3,hlNewColor(cs,255.0, 255.0, 128.0, 255.0, 255.0));
		hlParamSetNum(p3,100.0,100.0,200.0,200.0,0.5);
	hlImgPushNewOp(img,p3);
	hlParam *p4 = hlNewParam(HL_DRAW_CIRCLE);
		hlParamSetColor(p4,hlNewColor(cs,255.0, 0.0, 128.0, 255.0, 255.0));
		hlParamSetNum(p4,300.0,100.0,50.0,100.0,0.5);
	hlImgPushNewOp(img,p4);
	hlParam *p5 = hlNewParam(HL_DRAW_CIRCLE);
		hlParamSetColor(p5,hlNewColor(cs,0.0, 0.0, 0.0, 255.0, 255.0));
		hlParamSetNum(p5,100.0,300.0,50.0,55.0,1.0);
	hlImgPushNewOp(img,p5);
	int i = 100;
	float x,y,r;
	printf("yo\n");
	while(i--){
		x = (double)random()/(double)(RAND_MAX)*800;
		y = (double)random()/(double)(RAND_MAX)*600;
		r = (double)random()/(double)(RAND_MAX)*100;
		r += 2;
		p5 = hlNewParam(HL_DRAW_CIRCLE);
		hlParamSetColor(p5,c);
		hlParamSetNum(p5,x,y,r,r+1,(10.0/255.0));
		hlImgPushNewOp(img,p5);
	}
	printf("yo\n");
	hlRaw *Raw = hlNewRaw(cs,200,200);
	hlImgRenderToRaw(img,Raw,0,0,0,0);
	hlRawToPng(	Raw, "blending.png");
	i = 100;
	printf("yo\n");
	while(i--){
		x = (double)random()/(double)(RAND_MAX)*800;
		y = (double)random()/(double)(RAND_MAX)*600;
		r = (double)random()/(double)(RAND_MAX)*100;
		r += 2;
		p5 = hlNewParam(HL_DRAW_CIRCLE);
		hlParamSetColor(p5,c);
		hlParamSetNum(p5,x,y,r,r+1,(10.0/255.0));
		hlImgPushNewOp(img,p5);
	}
	printf("yo\n");
	Raw = hlNewRaw(cs,200,200);
	printf("yop\n");
	hlImgRenderToRaw(img,Raw,0,0,0,0);
	printf("yop\n");
	hlRawToPng(	Raw, "blending2.png");
	printf("yo\n");
	
	return 1;
}*/	
		
