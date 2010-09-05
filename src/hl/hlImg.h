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
	hlHash * statelib; 	/* to quickly find state top op */
	hlList * statelist;	/* list of valid states */
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
/**
 * Frees all resources used by the img.
 * @param img : the img that will be freed.
 */
void   hlFreeImg(hlImg* img);

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

/*------------- BBOX OPERATIONS ---*/

/** Adds an Open BBox to the operation stack. 
 *  Any operation pushed onto an open bbox will be put inside the box :
 *  The bbox will be modified so that its bounds fit the object.
 *  - Open BBox are always : on top of the oplist, in unsaved state
 *  @param img : the image
 *  @return : A reference to the BBox operation
 */
hlOpRef hlImgPushOpenBBox(hlImg *img,int rec_level, int max_depth, float max_ratio);
/** Close the deepest open bounding box.
 * @parm img : the image
 */
void  hlImgCloseBBox(hlImg *img);
/** Close all open BBox.
 * @param img : the image
 */
void hlImgCloseAllBBox(hlImg *img);

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
void	 hlGraphImg(FILE *f, const hlImg *img,int display);
/**
 * Returns the width in pixel of the current render region at zoom level z.
 * @param img 	: the image.
 * @param z	: the zoom level [0,31]
 * @return	: the width in pixel of the render region.
 */
int hlImgSizeX(hlImg *img, int z);
/**
 * Returns the height in pixel of the current render region at zoom level z.
 * @param img 	: the image.
 * @param z	: the zoom level [0,31]
 * @return	: the height in pixel of the render region.
 */
int hlImgSizeY(hlImg *img, int z);
/**
 * Returns the width in tiles of the current render region at zoom level z.
 * @param img 	: the image.
 * @param z	: the zoom level [0,31]
 * @return	: the width in tiles of the render region.
 */
int hlImgTileX(hlImg *img, int z);
/**
 * Returns the height in tiles of the current render region at zoom level z.
 * @param img 	: the image.
 * @param z	: the zoom level [0,31]
 * @return	: the height in tiles of the render region.
 */
int hlImgTileY(hlImg *img, int z);
/**
 * Returns the number of zoom levels in the image.
 * @param img 	: the image.
 * @return	: the zoom level where all the data is in a tile. [0,31]
 */
int hlImgDepth(hlImg *img);
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
			int tz	);
/* returns a modifiable tile from the rendered image */
hlTile * hlImgTileCopy( hlImg *img,
			hlState s,
			int tx,
			int ty,
			int tz	);
/* returns a raw rendering of the image in region and state */
hlRaw 	*hlImgRenderNewRaw(hlImg *img, hlState s, hlRegion r);
void hlImgRender(hlImg *img, hlState s, hlRaw*raw, int px, int py, int z);
void hlImgColorPick(hlImg *img, hlState s, int px, int py, int z, hlColor *color);


/** Renders a tile.
 * @param op : the operation that must be rendered.
 * @param cache : if 1 : the rendered tile is put in cache and the rendered tile is not
 *  returned. if 0 : a writable tile is returned. a copy of that tile may be put
 *  in cache. 
 * @param x,y,z : coordinates of the tile. z must be >= 0.
 */
hlTile *hlOpRenderTile(hlOp*op, int cache, int readonly, int sampling, int x, int y, int z);



#endif


