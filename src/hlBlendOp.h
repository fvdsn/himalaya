#ifndef __HL_BLENDING_OPERATION_H__
#define __HL_BLENDING_OPERATION_H__

#include "hlTile.h"
#include "hlOp.h"
void hl_blend_mix_8b(uint8_t *down, const uint8_t* up, int channel, float mix);


void hlBlendOp(hlTile *a, const hlTile *b, hlOp* p);
/* based on the parameters and the content of the tile *b,
 * decides if the blending operation has a) no effect,
 * b) doesn't need the data of the tile under *b
 * c) the operation should work as ususal.
 * 
 * a) it returns HL_INVISIBLE. the operation
 * 	has no effect, and the tile under *b can be returned
 * 	unmodified by the operation.
 * b) it returns HL_OPAQUE. you can then call
 *	hlBlendOpaque that can render the tile by itself. Rendering
 *	of tile *a can be skipped.
 * c) it returns HL_SEMITRANS and hlBlendOp should be used.
 */
int  hlBlendSkip(const hlTile *b, hlOp *p);
/* same as hlBlendOp but way faster and the original content of tile *a 
 * doesn't matter. Example : Blending the tile b on a with method = mix
 * and tile b is completely opaque. or method multiply and b is completely
 * black, etc... 
 */
void hlBlendOpaque(hlTile *a, const hlTile *b,hlOp *p);

#endif

