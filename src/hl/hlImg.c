#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "hlImg.h"
#include "hlBlendOp.h"
#include "hlDrawOp.h"
#define HL_OVERSAMPLING 1

extern int num_img;

hlImg* hlNewImg(hlColor color,int sx, int sy){
	hlImg* img = (hlImg*)malloc(sizeof(hlImg));
	img->top 	= NULL;
	img->statelib 	= hlNewHash(1009);
	img->statelist	= hlNewList();
	img->state 	= HL_STATE_UNSAVED;
	img->source 	= hlNewFrame(color,sx,sy);
	num_img++;
	return img;
}

hlImg* hlNewImgFromSource(hlFrame *frame){
	hlImg* img = (hlImg*)malloc(sizeof(hlImg));
	img->top 	= NULL;
	img->statelib 	= hlNewHash(1009);
	img->statelist	= hlNewList();
	img->state 	= HL_STATE_UNSAVED;
	img->source 	= frame;
	hlFrameMipMap(frame);	/*TODO remove that requirement */ 
	num_img++;
	return img;
}

/*------------- STATE -------------*/

static hlOp* hl_img_get_top_op(const hlImg *img, hlState state){
	if(state != HL_STATE_UNSAVED && state != HL_STATE_CURRENT){
		return (hlOp*)hlHashGet(img->statelib,state);
	}
	else {
		return img->top;
	}
}
static hlState hl_get_new_state(void){
	static int state = 1;
	return state++;
}

hlState hlImgStateSave(hlImg *img){
	hlState state = hl_get_new_state();
	hlOp * op = img->top;
	if(!op){
		return 0;
	}
	hlImgCloseAllBBox(img);
	hlHashAdd(img->statelib,(int)state,op);
	hlListAppend(img->statelist,(void*)state);
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
	//fprintf(stdout,"hlImgStateRem(...)");
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
	hlListRemAllData(img->statelist,(void*)remstate);
	return 0;
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
/*static void hl_img_close_box_op(hlOp *up,hlOp* op){
	while(op->down && op->down->type == HL_BBOX && op->down->open){
		up->down = op->down;
		op->down = op->down->down;
		up->down->down = op;
		up = up->down;
	}
	op->open = 0;
}*/

hlOpRef hlImgPushOp(hlImg *img, hlOp* op){
	hlOp *cur = img->top;
	hlOp *up = NULL;
	hlOp *dup = NULL;
	hlOpSetBBox(op);
	op->img = img;
	img->state = HL_STATE_UNSAVED; 
	hlOpSetCSIn(op,hlFrameCS(img->source));
	while(cur && cur->type == HL_BBOX && cur->open){
		if(op->type == HL_BBOX){
			if(op->rec_level > cur->rec_level){
				op->skip = cur->skip;
				op->depth++;
				break;
			}else if(op->rec_level == cur->rec_level-1){
				if(cur->max_depth && cur->depth +1 >= cur->max_depth){
					cur->depth++;
					cur->open = 0;
					op->skip = cur;
					op->down = cur;
					dup = cur;
					break;
				}else{
					cur->depth++;
				}
			}
		}else{
			if(cur->rec_level == 0){
				float wasted_area = 0.0f;
				hlBBox tmp = cur->bbox;
				hlBBoxExtend(&tmp,&(op->bbox));
				wasted_area = (hlBBoxArea(&tmp) - hlBBoxArea(&(cur->bbox))) / 
						(float)hlBBoxArea(&(op->bbox));
				cur->bbox = tmp;
				cur->depth++;
				cur->ratio += wasted_area;
				//printf("w:%f,%f\n",cur->ratio,cur->max_ratio);
				if((cur->depth >= cur->max_depth && cur->max_depth > 0)
					|| ((cur->ratio > cur->max_ratio) && cur->max_ratio > 0.0f)){
					cur->open = 0;
					dup = cur;
				}
			}else{
				hlBBoxExtend(&(cur->bbox),&(op->bbox));
			}
		}
		up = cur;
		cur = cur->down;
	}
	insert_op(up,op,cur);
	if(up == NULL){
		img->top    = op;
	}
	if(op->type == HL_BBOX && op->open && op->skip == NULL){
		op->skip = op->down;
		op->depth = 0;
	}
	if(dup){
		hlImgPushOpenBBox(img,dup->rec_level,dup->max_depth,dup->max_ratio);
	}
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
		return op;
	}
}
void hlImgModOpEnd(hlImg *img, hlOpRef ref){
	hlOp *op = hl_find_op(img,ref);
	if(!op){
		printf("ERROR: hlImgModOpEnd(...) operation not found.\n");
	}else{
		hlOpSetBBox(op);
	}
}

/*------------- BBOX -------------*/

hlOpRef hlImgPushOpenBBox(hlImg *img,int rec_level, int max_depth,float max_ratio){
	hlOp *op = hlNewOp(HL_BBOX_BOX);
	op->down = NULL;
	op->skip = NULL;
	op->open = 1;
	op->depth = 0;
	op->rec_level = rec_level;
	op->max_ratio = max_ratio;
	op->max_depth = max_depth;
	op->bbox.infinite = 0;
	return hlImgPushOp(img,op);
}
/*
hlOpRef hlImgPushOpenBBox(hlImg *img,int max_depth){
	hlOp *op = hlNewOp(HL_BBOX_BOX);
	hlOpRef ref = hlImgPushOp(img,op);
	op->skip = op->down;
	op->open = 1;
	op->depth = 0;
	op->max_depth = max_depth;
	op->bbox.infinite = 0;
	return ref;
}*/
void  hlImgCloseBBox(hlImg *img){
	hlOp *op = hl_img_get_top_op(img,HL_STATE_UNSAVED);
	while(op && op->type == HL_BBOX && op->refcount == 0 ){
		if(!op->down){
			op->open = 0;
			return;
		}else if(op->down->type != HL_BBOX){
			op->open = 0;
			return;
		}else if(op->down->open == 0){
			op->open = 0;
			return;
		}
		op = op->down;
	}
	printf("WARNING Cannot close BBox : Nothing to close\n");
}
void hlImgCloseAllBBox(hlImg *img){
	hlOp *op = hl_img_get_top_op(img,HL_STATE_UNSAVED);
	while(op && op->type == HL_BBOX){
		op->open = 0;
		op = op->down;
	}
}
/*------------- RENDER -------------*/

/* 	hlOpRenderTile(...) 		*/
static int hl_op_must_free_cache(hlOp *op, int x, int y, int z){
	return !op->caching;
}
static int hl_op_must_cache(hlOp *op, int x, int y, int z){
	return op->caching;
}
static hlTile *hl_op_render_adj(hlOp*op, int cache, int readonly, int osa, int x, int y, int z){
	const hlCS cs = hlOpGetCSIn(op);
	hlTile *tile = hlOpCacheGet(op,x,y,z);
	if(tile){	/* found in cache */
		if(readonly) {
			return tile;
		}else if(hl_op_must_free_cache(op,x,y,z)){
			return hlOpCacheRemove(op,x,y,z);
		}else{
			return hlTileDup(tile,cs);
		}
	}else{		/* not found in cache */
		if(op->down){
			tile = hlOpRenderTile(op->down,0,0,osa,x,y,z);
		}else{
			tile = hlFrameTileCopy(op->img->source,x,y,z);
		}
		if(!tile){
			printf("WARNING : Rendering NULL tile\n");
			tile = hlNewTile(cs);
		}
		op->render(tile,op);
		if(cache || hl_op_must_cache(op,x,y,z)){
			int cached = hlOpCacheSet(op,tile,cs,
					hlImgSizeX(op->img,0),
					hlImgSizeY(op->img,0), 
					x,y,z);
			//TODO tile can still leak in here ... :S
			if(readonly || !cached){
				return tile;
			}else{
				return hlTileDup(tile,cs);
			}
		}
		return tile;
	}
}
static hlTile *hl_op_render_group(hlOp*op, int cache, int readonly, int osa, int x, int y, int z){
	const hlCS cs = hlOpGetCSIn(op);
	hlTile *tile = hlOpCacheGet(op,x,y,z);
	if(tile){	/* found in cache */
		if(readonly) {
			return tile;
		}else if(hl_op_must_free_cache(op,x,y,z)){
			return hlOpCacheRemove(op,x,y,z);
		}else{
			return hlTileDup(tile,cs);
		}
	}else{		/* not found in cache */
		if(op->down){
			tile = hlOpRenderTile(op->down,0,0,osa,x,y,z);
		}else{
			tile = hlFrameTileCopy(op->img->source,x,y,z);
		}
		if(!tile){
			printf("WARNING : Rendering NULL tile\n");
			tile = hlNewTile(cs);
		}
		op->render(tile,op);
		if(cache || hl_op_must_cache(op,x,y,z)){
			int cached = hlOpCacheSet(op,tile,cs,
					hlImgSizeX(op->img,0),
					hlImgSizeY(op->img,0), 
					x,y,z);
			if(readonly || !cached){
				return tile;
			}else{
				return hlTileDup(tile,cs);
			}
		}
		return tile;
	}
}
static hlTile *hl_op_render_blend(hlOp*op, int cache, int readonly, int osa, int x, int y, int z){
	const hlCS cs = hlOpGetCSIn(op);
	hlTile *tile = hlOpCacheGet(op,x,y,z);
	const hlTile *tileup = NULL;
	if(tile){	/* found in cache */
		if(readonly) {
			return tile;
		}else if(hl_op_must_free_cache(op,x,y,z)){
			return hlOpCacheRemove(op,x,y,z);
		}else{
			return hlTileDup(tile,cs);
		}
	}else{		/* not found in cache */
		tileup = hlImgTileRead( 	hlOpGetAllImg(op)[0],
						hlOpGetAllState(op)[0],
						x,y,z);
		if(!tileup){
			printf("WARNING : Blending NULL tile \n");
			tileup = hlNewTile(cs);
		}
		if(op->down){
			tile = hlOpRenderTile(op->down,0,0,osa,x,y,z);
		}else{
			tile = hlFrameTileCopy(op->img->source,x,y,z);
		}
		if(!tile){
			printf("WARNING : Rendering NULL tile\n");
			tile = hlNewTile(cs);
		}
		hlBlendOp(tile,tileup,op);
		if(cache || hl_op_must_cache(op,x,y,z)){
			int cached = hlOpCacheSet(op,tile,cs,
					hlImgSizeX(op->img,0),
					hlImgSizeY(op->img,0), 
					x,y,z);
			if(readonly || !cached){
				return tile;
			}else{
				return hlTileDup(tile,cs);
			}
		}
		return tile;
	}
}
static hlTile *hl_op_render_bbox(hlOp*op, int cache, int readonly, int osa, int x, int y, int z){
	const hlCS cs = hlOpGetCSIn(op);
	hlTile *tile = NULL;
//	hlColor cinside = hlNewColor(hlOpGetCSIn(op),0.9,1.0,0.9,0,1.0);
//	hlColor cbox = hlNewColor(hlOpGetCSIn(op),0.99,0.99,1.0,0,1.0);
	
	/*if(!op->open){
		tile = hlOpCacheGet(op,x,y,z);
		if(tile){
			if(readonly){
				return tile;
			}else if(hl_op_must_free_cache(op,x,y,z)){
				return hlOpCacheRemove(op,x,y,z);
			}else{
				return hlTileDup(tile,cs);
			}
		}
	}*/
	if(hlBBoxTest(&(op->bbox),x,y,z)){ /* we are inside bbox */
		if(op->down){
			tile = hlOpRenderTile(op->down,cache,readonly,osa,x,y,z);
		}else{
			tile = hlFrameTileCopy(op->img->source,x,y,z);
		}
		//if(!readonly){
//			hlTileMult(tile,&cinside);
		//}
	}else{
		if(op->skip){
			tile = hlOpRenderTile(op->skip,cache,readonly,osa,x,y,z);
		}else{
			tile = hlFrameTileCopy(op->img->source,x,y,z);
		}
	}
//	if(!readonly){
//		hlTileMult(tile,&cbox);
//	}
	/*
	if(!op->open && (cache || hl_op_must_cache(op,x,y,z))){
		int cached = hlOpCacheSet(op,tile,cs,
				hlImgSizeX(op->img,0),
				hlImgSizeY(op->img,0), 
				x,y,z);
		if(readonly || !cached){
			return hlTileDup(tile,cs);
		}else{
			return hlTileDup(tile,cs);
		}
	}*/
	return tile;
}
static hlTile *hl_op_render_draw(hlOp*op, int cache, int readonly, int osa, int x, int y, int z){
	const hlCS cs = hlOpGetCSIn(op);
	hlTile *tile = hlOpCacheGet(op,x,y,z);
	if(tile){	/* found in cache */
		if(readonly) {
			return tile;
		}else if(hl_op_must_free_cache(op,x,y,z)){
			return hlOpCacheRemove(op,x,y,z);
		}else{
			return hlTileDup(tile,cs);
		}
	}else{		/* not found in cache */
		if(op->down){
			tile = hlOpRenderTile(op->down,0,0,osa,x,y,z);
		}else{
			tile = hlFrameTileCopy(op->img->source,x,y,z);
		}
		if(hlBBoxTest(&(op->bbox),x,y,z)){
			if(hlDrawOp(tile,op,x,y,z) && osa && z){
				/*hlTile  *t1,*t2,*t3,*t4;
				t1 = hlOpRenderTile(op,1,1,osa-1,x*2,  y*2,  z-1);
				t2 = hlOpRenderTile(op,1,1,osa-1,x*2+1,y*2,  z-1);
				t3 = hlOpRenderTile(op,1,1,osa-1,x*2+1,y*2+1,z-1);
				t4 = hlOpRenderTile(op,1,1,osa-1,x*2,  y*2+1,z-1);
				hlTileInterp(tile,cs,t1,t2,t3,t4);*/
			}
		}
		if(cache || hl_op_must_cache(op,x,y,z)){
			int cached = hlOpCacheSet(op,tile,cs,
					hlImgSizeX(op->img,0),
					hlImgSizeY(op->img,0), 
					x,y,z);
			if(readonly || !cached){
				return tile;
			}else{
				return hlTileDup(tile,cs);
			}
		}
		return tile;
	}
}
hlTile *hlOpRenderTile(hlOp*op, int cache, int readonly, int osa ,int x, int y, int z){
	switch (op->type){
		case HL_ADJUSTMENT:
			return hl_op_render_adj(op,cache,readonly,osa,x,y,z);
		case HL_BLENDING:
			return hl_op_render_blend(op,cache,readonly,osa,x,y,z);
		case HL_DRAW:
			return hl_op_render_draw(op,cache,readonly,osa,x,y,z);
		case HL_BBOX:
			return hl_op_render_bbox(op,cache,readonly,osa,x,y,z);
		case HL_GROUP:
			return hl_op_render_group(op,cache,readonly,osa,x,y,z);
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

void hlGraphImg(FILE *f,const hlImg *img,int display){
	hlOp *op = NULL;
	hlHash * drawn_node = hlNewHash(1009);
	hlHash * drawn_edge = hlNewHash(1009);
	if(!f){
		fprintf(stderr,"FAILURE : cannot print img graph on NULL file\n");
		return;
	}
	if(!img){
		fprintf(stderr,"WARNING : cannot print img graph of NULL img\n");
		return;
	}
	fprintf(f,"digraph img { \n rankdir = \"LR\"; size = \"20,100\";\n");
	fprintf(f,"img [ shape = record, color = green, label = \n");
	fprintf(f,"\"Img |<top> top | <state> state : %d | region: (...) | <statelib> statelib : %p | <source> source : %p \"];\n",
		img->state,(void*)img->statelib,(void*)img->source);
	/* source */
	if(img->source){
		fprintf(f,"frame_%p [ shape = record, color = blue, label = \"Frame | tilecount : %d \"];\n",
			(void*)img->source, hlFrameTileCount(img->source));
		fprintf(f,"img:source -> frame_%p [color = blue] \n",(void*)img->source);
	}	

	/* op nodes */
	op = img->top;
	while(op){
		hlGraphOp(f,op,display);
		fprintf(f,"\n");
		hlHashAdd(drawn_node,(int)op,op);
		op = op->down;
	}
	op = img->top;
	/* links between op nodes */
	while(op){
		if(op->down){
			hlHashAdd(drawn_edge,(int)op,op);
			fprintf(f," op_%p -> op_%p \n",(void*)op,(void*)op->down);
		}
		fprintf(f,"\n");
		op = op->down;
	}
	/* top link */
	if(img->top){
		fprintf(f,"img:top -> op_%p \n",(void*)img->top);
	}
	/* States */
	
	if(img->statelist){
		hlLNode *n = img->statelist->first;
		fprintf(f,"{rank = same;\n");
		while(n){
			fprintf(f,"state_%d;\n",(int)(n->data));
			n = n->next;
		}
		fprintf(f,"}\n");
		n = img->statelist->first;
		while(n){
			int s = (int)(n->data);
			fprintf(f,"img:statelib -> state_%d;\n",s);
			fprintf(f,"state_%d -> op_%p;\n",s,(void*)hlHashGet(img->statelib,s));
			n = n->next;
			op = hl_img_get_top_op(img,s);
			while(op && !hlHashGet(drawn_node,(int)op)){
				hlHashAdd(drawn_node,(int)op,op);
				hlGraphOp(f,op,display);
				fprintf(f,"\n");
				op = op->down;
			}
			op = hl_img_get_top_op(img,s);
			while(op && !hlHashGet(drawn_edge,(int)op)){
				if(op->down){
					hlHashAdd(drawn_edge,(int)op,op);
					fprintf(f," op_%p -> op_%p \n",(void*)op,(void*)op->down);
				}
				fprintf(f,"\n");
				op = op->down;
			}
		}
	}
	fprintf(f,"}\n");
}
/* 	hlImgSize_(...)	/ hlImgTile_(...)	*/
int hlImgSizeX(hlImg *img, int z){
	return hlFrameSizeX(img->source,z);
}
int hlImgSizeY(hlImg *img, int z){
	return hlFrameSizeY(img->source,z);
}
int hlImgTileX(hlImg *img, int z){
	return hlFrameTileX(img->source,z);
}
int hlImgTileY(hlImg *img, int z){
	return hlFrameTileY(img->source,z);
}
int hlImgDepth(hlImg *img){
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
hlTile *hlImgTileRead(hlImg *img, hlState state, int tx, int ty, int tz){
	hlOp *top = hl_img_get_top_op(img,state);
	hlTile *t = NULL;
	if(top){ 
		t = hlOpRenderTile(top,true,1,HL_OVERSAMPLING,tx,ty,tz);
	}else{
		t = hlFrameTileRead(img->source,tx,ty,tz);
	}
	if(!t){
		fprintf(stderr,"ERROR: could not read image tile; img:%p state:%d, tx:%d,ty:%d,tz:%d\n",
				(void*)img,state,tx,ty,tz);
	}
	return t;
}
hlTile *hlImgTileCopy(hlImg *img, hlState state, int x, int y, int z){
	return hlTileDup(hlImgTileRead(img,state,x,y,z),hlImgCS(img));
}
void hlImgColorPick(hlImg *img, hlState s, int px, int py, int z, hlColor *color){
	int tx = hlTileCoordFromCoord(px);
	int ty = hlTileCoordFromCoord(py);
	int rx = px - tx*HL_TILEWIDTH;
	int ry = py - ty*HL_TILEWIDTH;
	fprintf(stdout,"px,py:[%d,%d],tx,ty[%d,%d],rx,ry:[%d,%d],z:%d\n",px,py,tx,ty,rx,ry,z);
	hlTile *t = hlImgTileRead(img,s,tx,ty,z);
	hlTileColorPick(t,hlImgCS(img),rx,ry,color);
}

/*	hlImgToRaw(...) / hlImgToNewRaw(...) */
static void hl_tile_to_raw(hlTile *t, hlRaw *raw, int px, int py){
	const int sx = hlRawSizeX(raw);
	const int sy = hlRawSizeY(raw);
	const int bpp = hlCSGetBpp(hlRawCS(raw));
	uint8_t * tiledata = HL_DATA_8B(t);
	int ptx = 0;	/* top left point on tile */
	int pty = 0;
	int prx = px;	/* top left point on raw */
	int pry = py;
	int psx = HL_TILEWIDTH;	/* size of copy region */
	int psy = HL_TILEWIDTH;
	int y;
	if(px <= -HL_TILEWIDTH || py <= -HL_TILEWIDTH || px >= sx || py >=sy ||sx <= 0 || sy <=0){
		printf("rejected px,py:%d,%d\n",px,py);
		return;
	}
	if(px < 0){ 
		ptx = -px;
		psx += px;
		prx = 0;
	}
	if(py < 0){ 
		pty = -py;
		psy += py;
		pry = 0;
	}
	if(px + HL_TILEWIDTH > sx){
		psx -= (px + HL_TILEWIDTH - sx);
	}
	if(py + HL_TILEWIDTH > sy){
		psy -= (py + HL_TILEWIDTH - sy);
	}
	y = psy;
	if(psy <= 0 ){
		printf("rejected\n");
	}
	while(y-- > 0){
		memcpy(	raw->data +(prx+sx*(pry+y))*bpp,
			tiledata + (ptx+HL_TILEWIDTH*(pty+y))*bpp,
			psx*bpp);
	}
}
void hlImgRender(hlImg *img, hlState s, hlRaw *raw,int px, int py,int z){
	hlRegion r = hlNewRegion(px,py,hlRawSizeX(raw),hlRawSizeY(raw),z);
	hlTile *t = NULL;
	int x,y;	/*tile index*/
	int rx,ry;	/*top left pixel in raw where current tile will be copied */
	x = r.tx;	
	while(x--){
		y = r.ty;
		while(y--){
			rx = (r.ptx+x)*HL_TILEWIDTH -px;
			ry = (r.pty+y)*HL_TILEWIDTH -py;
			t = hlImgTileRead(img,s,r.ptx+x,r.pty+y, r.z );
			if(t){
				hl_tile_to_raw(t,raw,rx,ry);
			}
		}
	}
}
hlRaw *hlImgRenderNewRaw(hlImg *img, hlState s, hlRegion r){
	hlRaw *raw = hlNewRaw(hlImgCS(img),r.sx,r.sy);
	hlImgRender(img,s,raw,r.px,r.py,r.z);
	return raw;
}

	
		
