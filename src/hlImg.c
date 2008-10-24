#include <stdio.h>
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
	int caching;		/*always cache if true */
	int modifiable;		
	unsigned int refcount; /*count of saved state depending on this op*/
	unsigned int ref;	/*number unique to this operation, shared
				 *with operations duplicated from this one,
				 *because of modification of this op, or 
				 *a child one */
	unsigned int id;	/*what the operation does*/
	unsigned int type;	/*broad category of operation (blend,rot,...)*/
	unsigned int index;	/*height of operation from bottom*/
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
	img->top = NULL;
	img->region = hlNewRegion(0,0,sx,sy,0);
	img->statelib = hlNewHash(1009);
	img->state = HL_STATE_UNSAVED;
	img->source = hlNewFrame(color,sx,sy);
	return img;
}
hlImg* hlNewImgFromSource(hlFrame *frame){
	hlImg* img = (hlImg*)malloc(sizeof(hlImg));
	hl_debug_mem_alloc(HL_MEM_IMG);
	img->top = NULL;
	img->region = hlNewRegion(	0,0,
					hlFrameSizeX(frame,0),
					hlFrameSizeY(frame,0),
					0 			);
	img->statelib = hlNewHash(101);
	img->state = HL_STATE_UNSAVED;
	hlFrameMipMap(frame);
	img->source = frame;
	return img;
}
/* 	Cleaning Operations 	*/
hlOp* hl_img_purge_unsaved(hlOp *op){
/* frees any unsaved operation from and below op,
 * returns the first saved operation found */
	hlOp * tmp;
	while(op){
		tmp = op->down;
		if(op->refcount == 0){
			hlFreeOp(op);
		}
		else{
			return op;
		}
		op = tmp;
	}
	return NULL;
}
void hl_img_purge_cache(hlImg *img, hlOp * op){
	/* empties the caches of all operations above and equal index.
	 * this is used when you insert or remove an operation, the
	 * cached results above are not valid anymore and must be 
	 * purged */
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
}
/* 	State Operations 	*/
hlOp * hl_img_get_top_op(hlImg *img, unsigned int state){
	if(state){
		return (hlOp*)hlHashGet(img->statelib,state);
	}
	else {
		return img->top;
	}
}
hlState hl_get_new_state(){
	static int state = 0;
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

/* 	hlImgInsertOp(...) / hlImgRemoveOp(...) / etc... 	*/
hlOp* hlImgForkOp(hlImg *img, unsigned int ref_or_index, int useindex);
unsigned int hlImgGetOpCount(hlImg *img, hlState state){
	hlOp* top = hl_img_get_top_op(img,state);
	if(top){
		return top->index + 1;
	}
	else{
		return 0;
	}
}
hlOp* hl_img_get_op(hlImg *img, hlState state, unsigned int index){
	hlOp *op = hl_img_get_top_op(img,state);
	while(op){
		if(op->index == index){
			return op;
		}else if(op->index < index){
			return NULL;
		}
		op = op->down;
	}
	return NULL;
}
void insert_op(hlImg *img, hlOp *up, hlOp* newop, hlOp *down){
	if(newop == NULL){
		printf("WARNING : insert_op: inserting NULL op, nothing inserted\n");
		return;
	}
	if(up == NULL){ /*inserting top op*/
		if(down == NULL){ /*inserting op in empty list*/
			newop->down = NULL;
			newop->index = 0;
		}else{
			newop->down = down;
			newop->index = down->index + 1;
		}
		img->top = newop;
	}else{
		if(down == NULL){ /*inserting bottom op*/
			newop->down = NULL;
			newop->index = 0;
		}else{ /*inserting op in middle of op list */
			newop->down = down;
			newop->index = down->index + 1;
		}
		up->down = newop;
		assert(up->index == newop->index + 1);
	}
	hlParamLock(op->param);
	img->state = HL_STATE_UNSAVED;
	/*TODO check for forked states */
}
void hlImgInsertNewOp(hlImg *img, hlParam *p, unsigned int index){
	hlOp*newop = hlNewOp(p);
	hlOp*op = img->top;
	hlOp*op_up = NULL;

	while(op){
		if(op_up == NULL){ 		/*op == top op*/
			if(index > op->index){	/*inserting on top*/
				if(index > op->index + 1){
					printf("WARNING : hlImgInsertNewOp,"
						"index(%d) > opCount(%d), op "
						"inserted on top\n",
						index, op->index + 1);
				}
				newop->down = op;
				newop->index = op->index + 1;
				newop->img = img;
				img->top = newop;
				hlParam
	//TODO
	}}}}

void hlImgPopOp(hlImg *img){
	hlOp *tmp = NULL;
	if(img->top){
		tmp = img->top;
		img->top = tmp->down;
		if(tmp->refcount == 0){
			hlFreeOp(tmp);
		}
	}
	img->state = HL_STATE_UNSAVED;
}
void hlImgPushNewOp(hlImg *img, hlParam *p){
	hlOp* newop = hlNewOp(p);
	insert_op(img,NULL,newop,img->top);
}
/*	hl__ModOp(...) 		*/
/*if the current state is saved, and you want to modify a modop ,
 * you need to fork the oplist until you reach modop, and set
 * the new forked oplist as new list, with unsaved state*/
hlOp* hl_img_fork_saved_state(hlImg *img, unsigned int ref,int useindex){
	hlOp *op = img->top;
	hlOp *tmp = NULL;
	hlOp *newtop = NULL;
	assert(op);
	newtop = hlDupOp(op);
	if(	( useindex && newtop->index == ref) 
		|| (!useindex && newtop->ref == ref)){
		img->top = newtop;
		img->state = HL_STATE_UNSAVED;
		return newtop;
	}else{
		tmp = newtop;
		op = newtop->down;
		while(op){
			op = hlDupOp(op);
			tmp->down = op;
			if(op->ref == ref){
			if(	( useindex && op->index == ref) 
				|| (!useindex && op->ref == ref)	){
				img->top = newtop;
				img->state = HL_STATE_UNSAVED;
				return op;
			}
			else{
				op = op->down;
			}
		}
		printf("forked op not found in oplist, has forked anyway\n");
		img->top = newtop;
		img->state = HL_STATE_UNSAVED;
		return NULL;
	}
}
/* if the current state is unsaved, and you want to modify a modop, you
 * need to clean the cache of the higher op if they are unsaved(refcount == 0)
 * else, you need to fork them just as with saved state */
hlOp * hl_img_fork_unsaved_state(hlImg *img, unsigned int ref, int useindex){
	hlOp* u_op = img->top;
	hlOp* u_op_p = NULL;
	hlOp* s_op = NULL;
	if (u_op->refcount > 0){
		return hl_img_fork_saved_state(img,ref,useindex);
		
	}
	while(u_op && u_op->refcount == 0){
		hlOpCacheFree(u_op);
		u_op_p = u_op;
		u_op = u_op->down;
		if(	( useindex && u_op->index == ref) 
			|| (!useindex && u_op->ref == ref)	){
			return u_op;
		}
	}
	if(u_op){
		s_op = hlDupOp(u_op);
		u_op_p->down = s_op;
	}
	while(s_op){
		if(	( useindex && s_op->index == ref) 
			|| (!useindex && s_op->ref == ref)	){
			return s_op;
		}else if (s_op->down){
			s_op = hlDupOp(s_op->down);
		}else{
			printf("forked op not found in oplist, has forked anyway\n");
			return NULL;
		}
	}
	printf("serious fuck up in oplist discovered while forking it\n");
	return NULL;
}
hlOp* hlImgForkOp(hlImg *img, unsigned int ref, int useindex){
	if(img->state == HL_STATE_UNSAVED){
		return hl_img_fork_unsaved_state(img,ref,useindex);
	}else{
		return hl_img_fork_saved_state(img,ref,useindex);
	}
}

		

unsigned int hlImgPushNewModOp(hlImg *img, hlParam *p){
	hlOp *op = hlNewOp(p);
	op->modifiable = 1;
	hlImgPushOp(img,op);
	return op->ref;
}
hlOp *hl_find_op(hlImg *img, unsigned int ref){ /*TODO make it faster than O(n)*/
	hlOp* op = img->top;
	while(op){
		if (op->ref == ref){
			return op;
		}
		op = op->down;
	}
	return NULL;
}
hlParam * hlImgModOp(hlImg *img,unsigned int ref){
	hlOp * op = hl_find_op(img,ref);
	if(!op){
		printf("cannot modify non existing op\n");
		return NULL;
	}else if(!op->modifiable){
		printf("cannot modify non modifiable op\n");
		return NULL;
	}else{
		op = hl_img_fork_state(img,ref);
		hlParamUnlock(op->param);
		return op->param;
	}
}
void hlImgEndModOp(hlImg *img,unsigned int ref){
	hlOp *op = hl_find_op(img,ref);
	if(!op){
		printf("applying modifications on non existing op\n");
		return;
	}else if(!op->modifiable){
		printf("applying modifications on non modifiable op\n");
	}else{
		hlParamLock(op->param);
		hl_img_purge_cache(img,op);
	}
	return;
}

/* 	hlPrintImg(...) 	*/
void hlPrintImg(hlImg *img, hlState state){
	int i = hl_img_opcount(img,state);
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
uint32_t hlImgSizeX(hlImg *img, uint32_t z){
	return hlFrameSizeX(img->source,z);
}
uint32_t hlImgSizeY(hlImg *img, uint32_t z){
	return hlFrameSizeY(img->source,z);
}
uint32_t hlImgTileX(hlImg *img, uint32_t z){
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
hlOp* hlNewOp(hlParam *p){
	static unsigned int ref = 0;
	hlImg*img;
	hlState s;
	hlOp* op = (hlOp*)malloc(sizeof(hlOp));
	hl_debug_mem_alloc(HL_MEM_OPERATION);
	op->param = p;
	op->down = NULL;
	op->id 	 = hlParamGetId(p);
	op->ref = ref++;
	op->modifiable = 0;
	op->refcount = 0;
	op->caching = 0;
	op->cache = NULL;
	op->index = 0;
	assert(op_library[hlParamGetId(p)].id == op->id);
	op->render = op_library[hlParamGetId(p)].render;
	op->type = op_library[hlParamGetId(p)].type;
	/*when creating a blending op that links against another image
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
	dup->img = op->img;
	dup->down = op->down;
	dup->modifiable = op->modifiable;
	dup->index = op->index;
	dup->refcount = 0;
	dup->ref = op->ref;
	dup->caching = op->caching;
	dup->cache = NULL;
	dup->render = op->render;
	dup->type = op->type;
	dup->id = op->id;
	printf("dupOp\n");
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
	return;
}
void hlPrintOp(hlOp *op){
	printf("<Op> \n   id:%d\n   type:%d\n",op->id,op->type);
	if(op->caching){
		printf("   caching: true\n");
	}
	else{
		printf("   caching: false\n");
	}
	if(op->modifiable){
		printf("   modifiable: true\n");
	}
	else{
		printf("   modifiable: false\n");
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
		
