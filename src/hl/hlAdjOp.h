#ifndef __HL_ADJUSTEMENT_OPERATION_H__
#define __HL_ADJUSTEMENT_OPERATION_H__

#include "hlOp.h"
#include "hlTile.h"
void hlAdjOpSkip(hlTile *t, hlOp *op);
void hlAdjOpInvert(hlTile *t, hlOp *op);
void hlAdjOpFill(hlTile *t, hlOp *op);
void hlAdjOpMath(hlTile *t, hlOp *op);
void hlAdjOpChanmix(hlTile*t, hlOp *op);
void hlAdjOpLevels(hlTile*t, hlOp *op);
#endif

