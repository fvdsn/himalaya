#ifndef __HL_ADJUSTEMENT_OPERATION_H__
#define __HL_ADJUSTEMENT_OPERATION_H__

#include "hlParam.h"
#include "hlTile.h"
void hlAdjOpSkip(hlTile *t, hlParam *p);
void hlAdjOpInvert(hlTile *t, hlParam *p);
void hlAdjOpFill(hlTile *t, hlParam *p);
void hlAdjOpMath(hlTile *t, hlParam *p);
void hlAdjOpChanmix(hlTile*t, hlParam*p);
void hlAdjOpLevels(hlTile*t, hlParam*p);
#endif

