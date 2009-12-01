#ifndef __HL_IMG_H__
#define __HL_IMG_H__

#include <stdbool.h>
#include "hlOp.h"
#include "hlDataType.h"
#include "hlTile.h"
#include "hlFrame.h"
#include "hlADT.h"
#include "hlRegion.h"


struct hl_img{
	struct hl_op* top; 	/* last operation */
	hlState state;	/* unsaved, or the saved state id */
	hlRegion region;	
	hlHash * statelib; 	/* to quickly find state top op */
	hlFrame * source; 	/* original image data*/
};
void hlInit(void);
void hlStatPrint(void);

/**
 * Creates Img with bg color 'color', size sx,sy in pixels,
 * colorspace herited from the color,
 * @param c 	: the background color
 * @param sx 	: the width of the default render region in pixels
 * @param sy 	: the height of the default render region in pixels
 * @return 	: a new image filled will color c and with colorspace of c
 */
hlImg* hlNewImg(hlColor c, int sx, int sy);
/**
 * Creates Img from bitmap in frame. inherits all it's properties
 * @param frame : the frame used as the image background / source.
 * @return  	: a new image with a default render region the size of
 * 		the frame, with the same colorspace and contents.
 * 		You should not modify the source once you have set it.
 */
hlImg* hlNewImgFromSource(hlFrame *frame);
hlImg* hlNewImgFromRaw(hlRaw *raw);		/*TODO*/
hlImg* hlNewImgFromImg(hlImg *img, hlState s);	/*TODO*/
/**
 * Frees all resources used by the img.
 * @param img : the img that will be freed.
 */
void   hlFreeImg(hlImg* img);
void   hlLinkImg(hlImg* img);		/*TODO*/
void   hlUnlinkImg(hlImg* img);		/*TODO*/
void   hlLockImg(hlImg* img);		/*TODO*/
void   hlUnlockImg(hlImg *img);		/*TODO*/

/*------------- REGION -------------*/

/**
 * Changes the region to be rendered, the region can be bigger than the image. 
 * pixels outside will be rendered as transparent black 
 * @param img 	: the image with the new render region
 * @param r 	: the new render region.
 */
void 	 hlImgSetRegion(hlImg *img, hlRegion r);
/**
 * Returns the current render region of the image.
 * @param img 	: the image.
 * @return 	: the current region of img.
 */
hlRegion hlImgGetRegion(hlImg *img);

/*------------- OPERATION -------------*/

/**
 * Returns the operation count of a state.
 * @param img 	: the image.
 * @param state : a state of the image. 
 * @return 	: the count of operation in the image state.
 */
int 	hlImgGetOpCount(hlImg *img, hlState state);
/**
 * Puts a new operation on top of the operation list of the current
 * state. That means add an operation that will be executed after
 * all the previous that has been saved in the state. 
 * @param img 	: the image
 * @param op 	: the operation
 * @return 	: a reference that can be used to  find this operation in
 * a different saved state. 
 */
hlOpRef hlImgPushOp(hlImg *img, hlOp *op);
/**
 * Makes the current without the current top operation.  
 * @param img 	: the image
 * @return 	: the ref of the popped operation 
 */
hlOpRef hlImgPopOp(hlImg *img);
/** 
 * Returns the parameter of an operation in the current state
 * for direct modification access. 
 * @param img 	: the image
 * @param ref 	: the reference of the operation that will be modified.
 * @return 	: the operation that can be modified. Call hlImgEndModOp after
 * 		the operation has been modified to clean stuff up.
 */
hlOp* 	hlImgModOpBegin(hlImg *img, hlOpRef ref);
/**
 * Closes the operation modification
 * @param img : the image.
 * @parm ref : the modified operation OpRef
 */
void	hlImgModOpEnd(hlImg *img, hlOpRef ref);
/**
 * Marks the operation as having a very high probability
 * of beeing modified.
 * @param img : the image.
 * @param ref : the operation likely to be modified 
 */
void 	hlImgModOpHint(hlImg *img, hlOpRef ref);

/*------------- STATE -------------*/

#define HL_STATE_CURRENT 0 
#define HL_STATE_UNSAVED 0 /* the current unsaved image state */

/**
 * the default unsaved state is HL_STATE_UNSAVED. Saving 
 * the states returns an int that can be used as
 * a reference to return to the saved state.
 */
hlState hlImgStateSave(hlImg *img);
/**
 * returns to the saved state. if the new state does not exist, the
 * current state is kept, and it returns 0. 
 * unsaved changes are discareded.returns the old state.
 */ 
hlState hlImgStateLoad(hlImg *img, hlState s);
/**
 * returns the current state 
 */
hlState hlImgStateGet(hlImg *img);
/** 
 * returns a new state with the same data as original, but the original
 * can be safely removed without altering the new one
 */
hlState hlImgStateDup(hlImg *img, hlState s);
/**
 * returns 1 if the state exists 
 */
hlState hlImgStateExists(hlImg *img, hlState s);
/**
 * removes s
 */
hlState hlImgStateRem(hlImg *img, hlState s);

/*------------- INHERITED FROM HLFRAME -------------*/

/** 
 * Prints the internal representation of the image in the console
 * @param img 	: the image to be printed
 * @param s	: the state that will be printed.
 */
void 	 hlPrintImg(hlImg *img, hlState s);
/**
 * Returns the width in pixel of the current render region at zoom level z.
 * @param img 	: the image.
 * @param z	: the zoom level [0,31]
 * @return	: the width in pixel of the render region.
 */
uint32_t hlImgSizeX(hlImg *img, uint32_t z);
/**
 * Returns the height in pixel of the current render region at zoom level z.
 * @param img 	: the image.
 * @param z	: the zoom level [0,31]
 * @return	: the height in pixel of the render region.
 */
uint32_t hlImgSizeY(hlImg *img, uint32_t z);
/**
 * Returns the width in tiles of the current render region at zoom level z.
 * @param img 	: the image.
 * @param z	: the zoom level [0,31]
 * @return	: the width in tiles of the render region.
 */
uint32_t hlImgTileX(hlImg *img, uint32_t z);
/**
 * Returns the height in tiles of the current render region at zoom level z.
 * @param img 	: the image.
 * @param z	: the zoom level [0,31]
 * @return	: the height in tiles of the render region.
 */
uint32_t hlImgTileY(hlImg *img, uint32_t z);
/**
 * Returns the number of zoom levels in the image.
 * @param img 	: the image.
 * @return	: the zoom level where all the data is in a tile. [0,31]
 */
uint32_t hlImgDepth(hlImg *img);
/**
 * Returns the colorspace of the last operation in the current state.
 * @param img 	: the image.
 * @return	: the colorspace of the render in the current state.
 */
hlCS 	 hlImgCS(hlImg *img);
/**
 * returns a tile from the image for reading purpose, do not modify.
 * crashes if image hasn't yet been rendered 
 */
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
void hlImgRenderRegionToRaw(hlImg *img, hlRegion r, hlState s, hlRaw *raw); /*TODO*/

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


#endif


