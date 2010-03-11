#include <assert.h>
#include <stdio.h>
#include "hlImg.h"

static int hl_test_color_space(void){
	hlCS cs1 = hlNewCS(HL_8B,HL_RGB);
	hlCS cs2 = hlCSCopy(cs1);
	return 0;
}
static int hl_regr_black_tiles(void){
	hlCS cs = hlNewCS(HL_8B,HL_RGB);
	hlColor green 	= hlNewColor(cs,0,1,0,0,1);
	hlColor red 	= hlNewColor(cs,1,0,0,0,1);
	hlColor white 	= hlNewColor(cs,1,1,1,0,1);
	hlImg *img = hlNewImg(green,800,600);
	return 0;
}
static int hl_test_adt_list(void){
	fprintf(stdout,"TESTING: ADT LIST\n");
	int a,b,c,d,e;
	hlList *l = hlNewList();
	assert(l->size == 0);
	assert(hlListPop(l) == NULL);
	assert(hlListGet(l,0) == NULL);
	assert(hlListRemData(l, NULL) == 0);
	hlListPush(l,&a);
	assert(l->size == 1);
	assert(hlListGet(l,0) == &a);
	assert(hlListPop(l) == &a);
	assert(l->size == 0);
	hlListPush(l,&a);
	hlListPush(l,&b);
	assert(l->size == 2);
	assert(hlListGet(l,0) == &b);
	assert(hlListGet(l,1) == &a);
	assert(hlListPop(l) == &b);
	assert(l->size == 1);
	assert(hlListPop(l) == &a);
	assert(l->size == 0);
	hlListAppend(l,&a);
	hlListAppend(l,&b);
	hlListAppend(l,&c);
	hlListAppend(l,&d);
	hlListAppend(l,&e);
	hlListAppend(l,&c);
	assert(l->size == 6);
	assert(hlListGet(l,4) == &e);
	assert(hlListRemData(l,&c));
	assert(l->size == 5);
	assert(hlListRemData(l,&c));
	assert(l->size == 4);
	assert(hlListRem(l,2) == &d);
	assert(hlListRem(l,2) == &e);
	assert(hlListRem(l,0) == &a);
	assert(hlListRem(l,0) == &b);
	assert(l->size == 0);
	return 0;
}
static int hl_test_img(void){
 	hlCS cs	    = hlNewCS(HL_8B,HL_RGB);	
	hlRaw *in   = hlRawFromPng("images/girl.png");
	hlRaw *out  = hlNewRaw(cs,600,600);
	hlImg *img  = hlNewImgFromSource(hlFrameFromRaw(in));
	hlColor c   = hlNewColor(cs,0.5,1,0,0,0.9);
	hlImg *img2 = hlNewImg(c,10000,100000);
	hlOp *op;
	hlOpRef r1,r2,r3;

	hlInit();

	op = hlNewOp(HL_ADJ_MUL);
		hlOpSetAllValue(op,"factor",1.0,0.7,0.5,0.0,1.0);
	r1 = hlImgPushOp(img,op);
	
	op = hlNewOp(HL_DRAW_RECT);
		hlOpSetAllValue(op,"pos_tl",100.0,50.0);
		hlOpSetAllValue(op,"pos_br",400.0,500.0);
		hlOpSetAllValue(op,"alpha",0.5);
		hlOpSetAllColor(op,"fill_color",c);
	r2 = hlImgPushOp(img,op);

	hlState s1 = hlImgStateSave(img);
	
	op = hlImgModOpBegin(img,r2);
		hlOpSetAllColor(op,"fill_color",hlNewColor(cs,1.0,0.0,0.0,0.0,0.5));
		hlOpSetAllValue(op,"pos_br",200.0,500.0);
	hlImgModOpEnd(img,r2);

	op = hlNewOp(HL_DRAW_CIRCLE);
		hlOpSetAllValue(op,"pos_center",250.0,250.0);
		hlOpSetAllValue(op,"radius_in",100.0);
		hlOpSetAllValue(op,"radius_out",101.0);
		hlOpSetAllValue(op,"alpha",0.7);
		hlOpSetAllColor(op,"fill_color",c);
	hlImgPushOp(img,op);

	hlState s2 = hlImgStateSave(img);

	hlImgStateLoad(img,s1);

	hlImgRender(img,s1,out, 0, 0, 0);
	hlRawToPng( out, "out1.png");

	hlImgRender(img,s2,out, 0, 0, 0);
	hlRawToPng( out, "out2.png");

	hlGraphImg(fopen("graph1.grph","w"),img,HL_GRAPH_SIMPLE);
	return 0;
}
int main(int argc, char **argv){
	hl_test_adt_list();
	return 0;
}
