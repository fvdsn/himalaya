#include <stdint.h>
#include "hlColorSpace.h"
#include "hlColor.h"
#include "hlTile.h"
#include "hlColorSpace.h"
#include "hlColor.h"
#include "hlRaw.h"
#include "hlRegion.h"

#ifndef __HL_FRAME_H__
#define __HL_FRAME_H__

#define HL_MAX_TILE_LEVEL 32

typedef struct hl_frame hlFrame;
typedef struct hl_node{
	uint32_t x;
	uint32_t y;
	hlTile *tile;
	struct hl_node* tl;
	struct hl_node* tr;
	struct hl_node* br;
	struct hl_node* bl;
}hlNode;

struct hl_frame{
	hlRegion region;	/*when exporting to raw, export this region*/
	hlCS cs;
	hlColor color;		/*background color*/
	hlTile* bg;		/*tile filled with background color*/
	int depth; /* root node is depth 0 */
	struct hl_node *tlroot; /*(tx,ty) < 0*/
	struct hl_node *trroot; /*(tx>=0, ty <0)*/
	struct hl_node *brroot; /*(tx,ty) >=0*/
	struct hl_node *blroot; /*(tx<0, ty>=0)*/
};
/* WHAT IS A FRAME : 
 * hlFrame is a structure that contains bitmap data.
 * it doesn't have any size limits, and can return
 * tiles at different zoom levels.
 * It uses a quadtree to hold the tiles, each node
 * contains a mipmap tile of the four child nodes.
 * 
 * TODO: currently the creation of the mipmap nodes
 * only works if all the four child node exist.
 * 
 */
/*creates a new frame with background color bg, and size x,y in pixels*/
hlFrame *hlNewFrame(hlColor bg, int x, int y);
/*frees the frame and every tile in it*/
void 	hlFreeFrame(hlFrame *f);
/*frees every tile in the frame */
void 	hlFramePurge(hlFrame *f);
void 	hlPrintFrame(hlFrame *f);
/*returns the amount of tiles at the z level in the X/Y axis*/
int hlFrameTileX( 	hlFrame *f, int z);
int hlFrameTileY( 	hlFrame *f, int z);

/*returns the size in pixels in the X/Y axis*/
int hlFrameSizeX(hlFrame *f, int z);
int hlFrameSizeY(hlFrame *f, int z);

/*returns the count of mipmap levels */
int hlFrameDepth(hlFrame *f);

/*returns the frame background color*/
hlColor  hlFrameColor(hlFrame *f);

/*returns the frame colorspace*/
hlCS 	hlFrameCS(hlFrame *f);

/*replaces the tile at coordinates x,y,z, 
 * crashes if tile is NULL */
void 	hlFrameTileSet(	hlFrame *f, 
			hlTile *t, 
			int x, 
			int y,
			int z	);

/* returns the tile at coordinates x,y,z for UKNOWWHATUDO access
 * returns NULL if no tile */
hlTile 	*hlFrameTileGet( 	hlFrame *f,
				int x,
				int y,
				int z 	);

/* returns the tile at coordinates x,y,z for read only access,
 * returns a tile filled with the background color if empty, 
 * or out of bounds */
hlTile  *hlFrameTileRead(	hlFrame *f, 
				int x, 
				int y,
				int z	);

/* returns the tile at coordinates x,y,z for write access :
 * if the tile doesn't exists it inserts a new one and returns it */
hlTile 	*hlFrameTileWrite( 	hlFrame *f,
				int x,
				int y,
				int z 	);

/* returns a throw away copy of the tile at x,y,z for DWTFYW access,
 * don't forget to free it after use.
 * will return a copy of the tile with background color when needed,
 */
hlTile 	*hlFrameTileCopy( 	hlFrame *f,
				int x,
				int y,
				int z 	);

/* removes the corresponding tile from the frame and frees it.
 */
void 	hlFrameTileFree( 	hlFrame *f,
				int x,
				int y,
				int z	);
/* removes the corresponding tile from the frame and returns
 * it. will return NULL if tile didn't exist
 */
hlTile *hlFrameTileRemove(	hlFrame *f, 
				int x, 
				int y, 
				int z);
/* creates a raw with the entire frame contents at zoom level z */
hlRaw* 	hlRawFromFrame(hlFrame *f, int z);
hlRaw*  hlRawFromRegion(hlFrame *f, hlRegion r);
void    hlRegionToRaw(hlFrame *f, hlRaw *raw, int px, int py, int z);
/* creates a frame containing the raw as level 0. doesnt populates
 * tiles at higher zoom level */
hlFrame* hlFrameFromRaw(hlRaw *f);
void 	hlFrameMipMap(hlFrame *f);

#endif

