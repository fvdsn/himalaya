#ifndef __HL_IMG_H__
#define __HL_IMG_H__

#include <stdbool.h>
#include "hlDataType.h"
#include "hlTile.h"
#include "hlFrame.h"
#include "hlParam.h"
#include "hlADT.h"
#include "hlRegion.h"



#define HL_STATE_CURRENT 0 
#define HL_STATE_UNSAVED 0 /* the current unsaved image state */

/***********************************************+
 * 	Images functions 			|
 * *********************************************/

/* ############# IMG NEW #############*/
/* creates Img with bg color 'color', size sx,sy in pixels,
 * colorspace herited from the color */
hlImg* hlNewImg(hlColor c, int sx, int sy);

/*creates Img from bitmap in frame. inherits all it's properties*/
hlImg* hlNewImgFromSource(hlFrame *frame);
void   hlFreeImg(hlImg* img);

/* ############ REGION ###############*/
/* changes the region to be rendered, the region can be bigger than the image. 
 * pixels outside will be rendered as transparent black */
void 	 hlImgSetRegion(hlImg *img, hlRegion r);
hlRegion hlImgGetRegion(hlImg *img);

/* ###### OPERATION INSERTION #######*/
int hlImgGetOpCount(hlImg *img, hlState state);
/* Insert an operation in the image operation list. the new
 * operation has the specified index. Makes sure that tiles made
 * obsolete by the additions are removed, and makes sure that the top
 * operation has caching enabled. Disable caching for the old top
 * operation. 
 * Index 0 represents adding it to the bottom of the op list.
 * Index >= hlImgGetOpCount() add it at the top. sends warning
 * if Index > hlImgGetOpCount();
 */
void hlImgInsertNewOp(hlImg *img, hlParam *p,unsigned int index);

/* Removes the operation at specified index and frees it and all
 * it's data. Makes sure that tiles made obsolete by the removal are
 * purged from caches, and makes sure that the top operation has 
 * caching enabled. 
 * Index 0 removes bottom op. Index >= hlImgGetOpCount() - 1 removes
 * top op. WARNING if index >= hlImgGetOpCount()
 */
void hlImgRemoveOp(hlImg *img, unsigned int topindex);

/* puts an operation on top of the image operation list */
void hlImgPushNewOp(hlImg *img, hlParam *p);
/* puts a new blending operation on top of the image operation list */
void hlImgPushNewBlendOp(hlImg *img, int id, hlImg *up, float *alpha, float *mix);
/* removes the top operation, frees it and all its data */
void hlImgPopOp(hlImg *img);

/*----------------------------------------------+
 * 	Operation Modifications 		|
 *---------------------------------------------*/

hlParam * hlImgModOp(hlImg *img, unsigned int ref);
void 	  hlImgEndModOp(hlImg *img, unsigned int ref);
/* Inserts a modifiable operation in the img. The parameters of theses
 * operations can be changed and the image rerendered.*/
unsigned int hlImgPushNewModOp(hlImg *img, hlParam *p);

/*----------------------------------------------+
 * 	State Operations 			|
 *---------------------------------------------*/

/* the default unsaved state is HL_STATE_UNSAVED. Saving 
 * the states returns an int that can be used as
 * a reference to return to the saved state.*/
hlState hlImgStateSave(hlImg *img);
/* returns to the saved state. if the new state does not exist, the
 * current state is kept, and it returns 0. 
 * unsaved changes are discareded.returns the old state.*/ 
hlState hlImgStateLoad(hlImg *img, hlState s);
/* returns the current state */
hlState hlImgStateGet(hlImg *img);
/* returns a new state with the same data as original, but the original
 * can be safely removed without altering the new one*/
hlState hlImgStateDup(hlImg *img, hlState s);
/* returns 1 if the state exists */
hlState hlImgStateExists(hlImg *img, hlState s);
/* removes s*/
hlState hlImgStateRem(hlImg *img, hlState s);

void hlPrintImg(hlImg *img, hlState s);

/* functions inherited from hlFrame : */
uint32_t hlImgSizeX(hlImg *img, uint32_t z);
uint32_t hlImgSizeY(hlImg *img, uint32_t z);
uint32_t hlImgTileX(hlImg *img, uint32_t z);
uint32_t hlImgTileY(hlImg *img, uint32_t z);
uint32_t hlImgDepth(hlImg *img);
hlCS 	 hlImgCS(hlImg *img);
/* returns a tile from the image for reading purpose, do not modify.
 * crashes if image hasn't yet been rendered */
hlTile * hlImgTileRead(	hlImg *img,
			hlState s,
			int tx,
			int ty,
			unsigned int tz	);
/* returns a modifiable tile from the rendered image */
hlTile * hlImgTileCopy( hlImg *img,
			hlState s,
			int tx,
			int ty,
			unsigned int tz	);
/* makes sure the selected tile is rendered and up to date */
void hlImgRenderTile(	hlImg *img,
			hlState s,
			int tx,
			int ty,
			unsigned int tz	);
/*renders all the tiles in the region */
void hlImgRenderRegion(hlImg *img, hlRegion r, hlState s);
/* returns a frame for reading purpose valid until next modification
 * of image */
hlFrame *hlImgReadFrame(hlImg *img, hlState s);
/* returns a raw rendering of the image in region and state */
hlRaw 	*hlImgRenderNewRaw(hlImg *img, hlRegion r, hlState s);
void hlImgRenderToRaw(hlImg *img, hlRaw*raw, hlState state, int px, int py, unsigned int z);

/***********************************************+
 * 	Operation functions 			|
 * *********************************************/

hlOp* 	hlNewOp(hlParam *p);
hlOp* 	hlDupOp(hlOp *p);
void 	hlFreeOp(hlOp* op);
void 	hlPrintOp(hlOp *op);
/*if caching is enabled for an operation, a copy of every tile produced
 * by the operation is put in the cache. tiles in the cache should always be up to date. */

void	hlOpCacheEnable(hlOp*op);

/*when caching is disabled , every cache access results in the removal from the corresponding tile in the cache */
void 	hlOpCacheDisable(hlOp*op);

/*removes every tile in the cache and frees it*/
void 	hlOpCacheFree(hlOp*op);

/*removes the tiles from the cache and returns it*/
hlTile *hlOpCacheRemove(hlOp* op, int x, int y, unsigned int z);

/*places a copy of the tile in the cache. frees the original one,
 * if it exists. tile should not be NULL. WARNING : any modification of
 * the tile after it has been set will modify the tile in the cache 
 * as well, use hlTileDup() */
void 	hlOpCacheSet(hlOp* op, hlTile*tile, int tx, int ty,unsigned int tz);

/*returns the tile in the cache. returns NULL if there is no tile in
 * the cache. WARNING any modification of the returned tile will modify
 * the cache ass well. use hlTileDup() if you plan to modify the tile*/
hlTile *hlOpCacheGet(hlOp* op, int tx, int ty, unsigned int tz);

/* renders the tile in the designed op and below. if an operation
 * has caching enabled, a copy of the result is put into the tile.
 * if an operation has caching disabled and the corresponding tile
 * is found in cache, it is returned and removed from the cache,
 * 
 * the function always returns a modifable tile containing the render,
 * unless it is called on a top operation, then it returns a tile
 * that _should not_ be modified.
 *
 * this function is called by image render API on the top operation, 
 * then it calls itself recursively, it represents the core rendering
 * algorithm. 
 * */
hlTile *hlOpRenderTile(hlOp* op, bool istop, int tx, int ty, unsigned int tz);

/***********************************************+
 * 	Operation Framework 			|
 * *********************************************/

#endif


