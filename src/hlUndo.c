#include<stdlib.h>
#include<stdio.h>
#include"hlUndo.h"

struct hl_undo_s{
	hlImg *img;
	int maxlevel;       /*maximum undo level on stack  level <= maxlevel*/
	int level;          /*the maximum level used  currentlevel <= level*/
	int currentlevel;   /*the level you get if you press undo >= 0*/
	hlState *undostate; /*[maxlevel]*/
};

hlUndo * hlNewUndo(hlImg *img,int maxlevel){
	hlUndo *u = (hlUndo*)malloc(sizeof(hlUndo));
	if(maxlevel <=0){
		maxlevel = HL_DEFAULT_MAX_UNDO;
	}
	u->img = img;
	u->maxlevel = maxlevel;
	u->level = 0;
	u->currentlevel = 0;
	u->undostate = (hlState*)malloc(maxlevel*sizeof(hlState));
	u->undostate[0] = hlImgStateSave(img);
	return u;
}
hlState hlUndoPush(hlUndo *u){
	int i = 0;
	printf("undo: PUSH\n");
	if(u->currentlevel == u->maxlevel -1){ /*undo stack full */
		printf("\tundo stack full\n");
		hlImgStateRem(u->img,u->undostate[0]);
		i = 1;
		while(i < u->maxlevel){
			u->undostate[i-1] = u->undostate[i];
		}
		return (u->undostate[u->currentlevel] = hlImgStateSave(u->img));
	}else if(u->currentlevel < u->level){ /*there is redo levels*/
		printf("\tthere is redo levels\n");
		printf("\tcurr.lvl:%d, level:%d\n",u->currentlevel,u->level);
		i = u->currentlevel;
		while(++i <= u->level){
			hlImgStateRem(u->img,u->undostate[i]);
		}
		u->currentlevel++;
		u->level = u->currentlevel;
		printf("\tredo levels removed\n");
		printf("\tcurr.lvl:%d, level:%d\n",u->currentlevel,u->level);
		return (u->undostate[u->currentlevel] = hlImgStateSave(u->img));
	}else{				  /* no redo levels */
		printf("\tadding new level\n");
		u->level++;
		u->currentlevel++;
		printf("\tcurr.lvl:%d, level:%d\n",u->currentlevel,u->level);
		return (u->undostate[u->currentlevel] = hlImgStateSave(u->img));
	}
}		
hlState hlUndoUndo(hlUndo *u){
	printf("undo: UNDO \n");
	if(hlImgStateGet == HL_STATE_UNSAVED){
		printf("\tstate unsaved\n");
		hlImgStateLoad(u->img,u->undostate[u->currentlevel]);
		printf("\tcurr.lvl:%d, level:%d\n",u->currentlevel,u->level);
		return u->undostate[u->currentlevel];
	}else if(u->currentlevel == 0){
		printf("\tno undo possible\n");
		printf("\tcurr.lvl:%d, level:%d\n",u->currentlevel,u->level);
		return 0;
	}else {
		printf("\tdown one level\n");
		u->currentlevel--;
		printf("\tcurr.lvl:%d, level:%d\n",u->currentlevel,u->level);
		hlImgStateLoad(u->img,u->undostate[u->currentlevel]);
		return u->undostate[u->currentlevel];
	}
}
hlState hlUndoRedo(hlUndo *u){
	printf("undo: UNDO \n");
	if(u->currentlevel >= u->level){
		printf("\tno redo possible\n");
		printf("\tcurr.lvl:%d, level:%d\n",u->currentlevel,u->level);
		return 0;
	}else{
		printf("\tup one level\n");
		u->currentlevel++;
		printf("\tcurr.lvl:%d, level:%d\n",u->currentlevel,u->level);
		hlImgStateLoad(u->img,u->undostate[u->currentlevel]);
		return u->undostate[u->currentlevel];
	}
}

