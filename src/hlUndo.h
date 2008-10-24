#ifndef __HL_UNDO_H__
#define __HL_UNDO_H__
#include "hlImg.h"

#define HL_DEFAULT_MAX_UNDO 32
typedef struct hl_undo_s hlUndo;

hlUndo * hlNewUndo(hlImg *img,int maxlevel);
void    hlFreeUndo(hlUndo *u, int freestates);
hlState hlUndoPrePush(hlUndo *undo);
hlState hlUndoPush(hlUndo *undo);
hlState hlUndoUndo(hlUndo *undo);
hlState hlUndoRedo(hlUndo *undo);
void    hlUndoClear(hlUndo *undo);
#endif

