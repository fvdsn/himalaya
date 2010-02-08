#ifndef __HL_DRAW_OP_H__
#define __HL_DRAW_OP_H__
#include "hlTile.h"
#include "hlOp.h"

void hlDrawOp(hlTile *a, hlOp *p,int tx, int ty, int tz);
/* based on the parameters and the location of the tile to be
 * rendered
 * decides if the blending operation has a) no effect,
 * b) doesn't need the data of the tile under *a
 * c) the operation should work as ususal.
 * 
 * a) it returns HL_INVISIBLE. the operation
 * 	has no effect, and the tile *a can be returned
 * 	unmodified by the operation.
 * b) it returns HL_OPAQUE. you can then call
 *	hlDrawOpaque that can render the tile by itself. Rendering
 *	of tile *a can be skipped.
 * c) it returns HL_SEMITRANS and hlDrawOp should be used.
 */
int  hlDrawSkip(hlOp *p,int tx, int ty, int tz );
/* Same as hlDrawOp but way faster and discards the data in *a so it doesn't
 * have to be rendered. Example : rendering a tile inside an opaque
 * rectangle
 */
void hlDrawOpaque(hlTile *a, hlOp *p, int tx, int ty, int tz);
void hlSquareBBoxFun(const hlOp *op, hlBBox *box);
void hlCircleBBoxFun(const hlOp *op, hlBBox *box);
#endif
