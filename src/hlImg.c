#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "hlImg.h"
#include "hlBlendOp.h"
#include "hlDrawOp.h"

struct hl_img{
	struct hl_op* top; 	/* last operation */
	hlState state;	/* unsaved, or the saved state id */
	hlRegion region;	
	hlHash * statelib; 	/* to quickly find state top op */
	hlFrame * source; 	/* original image data*/
};

hlImg* hlNewImg(hlColor color,int sx, int sy){
	hlImg* img = (hlImg*)malloc(sizeof(hlImg));
	img->top 	= NULL;
	img->region 	= hlNewRegion(0,0,sx,sy,0);
	img->statelib 	= hlNewHash(1009);
	img->state 	= HL_STATE_UNSAVED;
	img->source 	= hlNewFrame(color,sx,sy);
	return img;
}

hlImg* hlNewImgFromSource(hlFrame *frame){
	hlImg* img = (hlImg*)malloc(sizeof(hlImg));
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
	hlOpCacheEnable(op,1);
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
					hlFreeOp(op);	/*TODO remove referenced states */
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
	hlOpCacheEnable(op,0);
	while(op){
		tmp = op->down;
		op->refcount--;
		if(op->refcount == 0){
			hlFreeOp(op);	/* TODO remove referenced states */
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
	hlOpLock(newop);
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
			hlFreeOp(tmp);	/* TODO remove referenced states */
		}
		return ref;
	}else{
		printf("WARNING: hlImgPopOp(...) : nothing to pop.\n");
		return 0;
	}
}
hlOpRef hlImgPushOp(hlImg *img, hlOp* op){
	if(img->top){
		hlOpSetCSIn(op,hlOpGetCSIn(img->top));
	}else{
		hlOpSetCSIn(op,hlFrameCS(img->source));
	}
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
hlOp * hlImgModOpBegin(hlImg *img,hlOpRef ref){
	hlOp * op = hl_find_op(img,ref);
	if(!op){
		printf("ERROR: hlImgModOpBegin(...) operation not found.\n");
		return NULL;
	}else{
		op = hlImgForkOp(img,ref);
		hlOpUnlock(op);
		return op;
	}
}
void hlImgModOpEnd(hlImg *img, hlOpRef ref){
	hlOp *op = hl_find_op(img,ref);
	if(!op){
		printf("ERROR: hlImgModOpEnd(...) operation not found.\n");
	}else{
		hlOpLock(op);
	}
}

/* 	hlOpRenderTile(...) 		*/
hlTile *hl_op_render_adj(hlOp* op, bool top, int x, int y, unsigned int z){
	const hlCS cs = hlOpGetCSIn(op);
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
		op->render(tile,op);
		if(op->caching || top ){ 	/*put tile in cache*/
			if(top){
				hlOpCacheSet(op,tile,cs,
						hlImgSizeX(op->img,0),
						hlImgSizeY(op->img,0),
						x,y,z);
				return tile;
			}
			else{
				hlOpCacheSet(op,tile,cs,
						hlImgSizeX(op->img,0),
						hlImgSizeY(op->img,0),
						x,y,z);
				return hlTileDup(tile,cs);
			}
		}
		else{
			return tile;
		}
	}
}
hlTile *hl_op_render_blend(hlOp* op, bool top, int x, int y, unsigned int z){
	const hlCS cs = hlOpGetCSIn(op);
	hlTile *tile = hlOpCacheGet(op,x,y,z);
	const hlTile *tileup = hlImgTileRead(	hlOpGetAllImg(op)[0],
					hlOpGetAllState(op)[0],
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
		hlBlendOp(tile,tileup,op);
		if(op->caching || top ){ 	/*put tile in cache*/
			if(top){
				hlOpCacheSet(	op,tile,cs,
						hlImgSizeX(op->img,0),
						hlImgSizeY(op->img,0),
						x,y,z);
				return tile;
			}
			else{	/* if not top operation, the tile will
				be modified later, so we return a copy
				of the tile */
				hlOpCacheSet(	op,tile,cs,
						hlImgSizeX(op->img,0),
						hlImgSizeY(op->img,0),
						x,y,z);
				return hlTileDup(tile,cs);
			}
		}
		else{
			return tile;
		}
	}
}
hlTile *hl_op_render_draw(hlOp* op, bool top, int x, int y, unsigned int z){
	hlCS cs = hlOpGetCSIn(op);
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
		hlDrawOp(tile,op,x,y,z);
		if(op->caching || top ){ 	/*put tile in cache*/
			if(top){
				hlOpCacheSet(op,tile,cs,
						hlImgSizeX(op->img,0),
						hlImgSizeY(op->img,0),
						x,y,z);
				return tile;
			}
			else{
				hlOpCacheSet(op,tile,cs,
						hlImgSizeX(op->img,0),
						hlImgSizeY(op->img,0),
						x,y,z);
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

/*------------- IMAGE -------------*/

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
		return hlOpGetCSIn(img->top);
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

int main(int argc, char **argv){
 	hlCS cs	    = hlNewCS(HL_8B,HL_RGB);	
	hlRaw *in   = hlRawFromPng("images/girl_.png");
	hlRaw *out  = hlNewRaw(cs,600,600);
	hlImg *img  = hlNewImgFromSource(hlFrameFromRaw(in));
	hlColor c   = hlNewColor(cs,0.5,1,0,0,0.9);
	hlOp *op;
	hlOpRef r;

	hlInit();

	op = hlNewOp(HL_ADJ_MUL);
		hlOpSetAllValue(op,"factor",1.0,0.7,0.5,0.0,1.0);
	hlImgPushOp(img,op);
	
	op = hlNewOp(HL_DRAW_RECT);
		printf("%p\n",op);
		hlOpSetAllValue(op,"pos_tl",100.0,50.0);
		hlOpSetAllValue(op,"pos_br",400.0,500.0);
		hlOpSetAllValue(op,"alpha",0.5);
		hlOpSetAllColor(op,"fill_color",c);
	r = hlImgPushOp(img,op);

	hlState s1 = hlImgStateSave(img);
	printf("%d\n",s1);
	
	op = hlImgModOpBegin(img,r);
		printf("%p\n",op);
		hlOpSetAllColor(op,"fill_color",hlNewColor(cs,1.0,0.0,0.0,0.0,0.5));
		hlOpSetAllValue(op,"pos_br",200.0,500.0);
	hlImgModOpEnd(img,r);

	hlState s2 = hlImgStateSave(img);
	printf("%d\n",s2);

	hlImgStateLoad(img,s1);

	hlImgRenderToRaw(img, out,s1, 0, 0, 0);
	hlRawToPng( out, "out1.png");

	hlImgRenderToRaw(img, out,s2, 0, 0, 0);
	hlRawToPng( out, "out2.png");

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
		
