#include <stdint.h>
#include "hlColorSpace.h"
#include "hlColor.h"

#ifndef __HL_TILE_H__
#define __HL_TILE_H__

/* OOKAY this is extremely ugly. basically i wanted a 
 * structure with variable size. you can access
 * the data as it would be in the data[] table
 * but C doesn't allow a structure with only an undefined array.
 *
 * Other solutions should be possible.
 */
typedef struct hl_tile{
	char data[1];
	char end[];
}hlTile;

enum hl_tilesize_8b { /*numbers based on tilewidth = 32 */
	HL_TILEWIDTH_8B1 = 1024,
	HL_TILEWIDTH_8B2 = 2048,
	HL_TILEWIDTH_8B3 = 3072,
	HL_TILEWIDTH_8B4 = 4096,
	HL_TILEWIDTH_8B5 = 5120
};
enum hl_tilesize_32b {
	HL_TILEWIDTH_32B1 = 4096,
	HL_TILEWIDTH_32B2 = 8192,
	HL_TILEWIDTH_32B3 = 12288,
	HL_TILEWIDTH_32B4 = 16384,
	HL_TILEWIDTH_32B5 = 20480
};

#define HL_TILEWIDTH    32
#define HL_TILE_PIXCOUNT HL_TILEWIDTH*HL_TILEWIDTH
#define HL_DATA_8B(tile) ((uint8_t*)(tile->data))
#define HL_DATA_32B(tile) ((float*)(tile->data))
#define HL_XY(x,y,chan) ((HL_TILEWIDTH*(y)+(x))*(chan))
#define HL_XYC(x,y,chan) (HL_XY((x),(y),(chan)) + (chan))
#define HL_INDEX(i,chan) ((i)*(chan))

hlTile *hlNewTile(hlCS cs);
void    hlFreeTile(hlTile *tile);
int  	hlTileSize(hlCS cs);
void 	hlTileZeroes(hlTile *tile, hlCS cs);
void 	hlTileRandom(hlTile *tile, hlCS cs);
void 	hlTileFill(hlTile *tile, hlColor *color);
void	hlTileMult(hlTile *tile,hlColor *color);
void 	hlTileCopy(hlTile *dst, const hlTile *src, hlCS cs);
void	hlTileInterp(hlTile *dst, hlCS cs,const hlTile *tl, const hlTile *tr, 
			const hlTile *br, const hlTile *bl);
hlTile *hlTileDup(const hlTile *src, hlCS cs);
float 	hlTileGetXYC(	hlTile *tile,
			hlCS cs,
			unsigned int X, 
			unsigned int Y, 
			unsigned int Chan);
void 	hlPrintTile(hlTile *tile, hlCS cs);
void 	hlPrintTileMem(hlTile *tile,hlCS cs);

#endif

