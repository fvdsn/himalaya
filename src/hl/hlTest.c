#include "hlImg.h"
int main(int argc, char **argv){
 	hlCS cs	    = hlNewCS(HL_8B,HL_RGB);	
	hlRaw *in   = hlRawFromPng("images/girl.png");
	hlRaw *out  = hlNewRaw(cs,600,600);
	hlImg *img  = hlNewImgFromSource(hlFrameFromRaw(in));
	hlColor c   = hlNewColor(cs,0.5,1,0,0,0.9);
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

	hlImgRenderToRaw(img, out,s1, 0, 0, 0);
	hlRawToPng( out, "out1.png");

	hlImgRenderToRaw(img, out,s2, 0, 0, 0);
	hlRawToPng( out, "out2.png");

	return 0;
}
