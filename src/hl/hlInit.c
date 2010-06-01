#include <stdlib.h>
#include "hlImg.h"
#include "hlOp.h"
#include "hlAdjOp.h"
#include "hlDrawOp.h"

extern hlOpClass *op_library;

void hlInit(void){
	hlOpClass *c;


	/* -------------- BBOX OPERATION ----------- */

	c = hlNewOpClass(	"bbox",	"a bounding box",	
				HL_BBOX_BOX,	HL_BBOX,	NULL);		
	/* -------------- ADJUSTMENT OPERATION ----------- */

	c = hlNewOpClass(	"skip",	"this operation does nothing",	
				HL_ADJ_SKIP,	HL_ADJUSTMENT,	hlAdjOpSkip	);		/*func*/
	
	c = hlNewOpClass(	"invert","the pixel value are 1.0 - val",
				HL_ADJ_INVERT,	HL_ADJUSTMENT,	hlAdjOpInvert	);

	c = hlNewOpClass(	"fill",	"fills the image with a color",
				HL_ADJ_FILL,	HL_ADJUSTMENT,	hlAdjOpFill	);
	hlOpClassAddColor(	c, "fill_color", "the fill color", 1		);

	c = hlNewOpClass(	"test",	"tests the performance of the renderer",
				HL_ADJ_TEST,	HL_ADJUSTMENT, 	NULL	);
	hlOpClassAddColor(	c, "test_color", "testing param", 2		);
	hlOpClassAddNum(	c, "test_num", "testing param", 2, 0.0, 1.0	);

	c = hlNewOpClass(	"add",	"adds value to the pixels",
				HL_ADJ_ADD,	HL_ADJUSTMENT,	hlAdjOpMath	);
	hlOpClassAddNum(	c, "factor", "the additive factors", 5, -255, 255 );

	c = hlNewOpClass(	"mul",	"multiplies the pixel with constant factors",
				HL_ADJ_MUL,	HL_ADJUSTMENT,	hlAdjOpMath	);
	hlOpClassAddNum(	c, "factor", "the product factors", 5, -255, 255 );

	c = hlNewOpClass(	"div",	"divides the pixels with constant factors",
				HL_ADJ_DIV,	HL_ADJUSTMENT,	hlAdjOpMath	);
	hlOpClassAddNum(	c, "factor", "the division factors", 5, -255, 255 );

	c = hlNewOpClass(	"mod",	"modulo operation on the pixels",
				HL_ADJ_MOD,	HL_ADJUSTMENT,	hlAdjOpMath	);
	hlOpClassAddNum(	c, "factor", "the modulo factors", 5, -255, 255 );

	c = hlNewOpClass(	"less",	"minimum value of pixel and factor",
				HL_ADJ_LESS,	HL_ADJUSTMENT,	hlAdjOpMath	);
	hlOpClassAddNum(	c, "factor", "the max factors", 5, -255, 255 );

	c = hlNewOpClass(	"more",	"maximum value of pixel and factor",
				HL_ADJ_MORE,	HL_ADJUSTMENT,	hlAdjOpMath	);
	hlOpClassAddNum(	c, "factor", "the min factors", 5, -255, 255 );

	c = hlNewOpClass(	"pow",	"takes the power of pixels values",
				HL_ADJ_POW,	HL_ADJUSTMENT,	hlAdjOpMath	);
	hlOpClassAddNum(	c, "factor", "the power factors", 5, -255, 255 );

	c = hlNewOpClass(	"chanmix",	"mixes the values of the channels",
				HL_ADJ_CHANMIX,	HL_ADJUSTMENT,	NULL /*TODO*/	);
	hlOpClassAddNum(	c, "factor_matrix", "the matrix of mix factors", 25, -1, 1 );

	/* -------------- BLENDING OPERATION ----------- */

	c = hlNewOpClass(	"blend_mix",	"blend two images with mix blending mode",
				HL_BLEND_MIX,	HL_BLENDING,	NULL	);
	hlOpClassAddNum(	c, "alpha", 	"linear blending factor", 1, 0,1 );
	hlOpClassAddNum(	c, "mix", 	"nonlinear blending factor", 1, 0,1 );
	hlOpClassAddImage(	c, "up_image",	"the up image", 1);

	c = hlNewOpClass(	"blend_add",	"blend two images with additive blending mode",
				HL_BLEND_ADD,	HL_BLENDING,	NULL	);
	hlOpClassAddNum(	c, "alpha", 	"linear blending factor", 1, 0,1 );
	hlOpClassAddNum(	c, "mix", 	"nonlinear blending factor", 1, 0,1 );
	hlOpClassAddImage(	c, "up_image",	"the up image", 1);
	
	c = hlNewOpClass(	"blend_mult",	"blend two images with multiplicative blending mode",
				HL_BLEND_MULT,	HL_BLENDING,	NULL	);
	hlOpClassAddNum(	c, "alpha", 	"linear blending factor", 1, 0,1 );
	hlOpClassAddNum(	c, "mix", 	"nonlinear blending factor", 1, 0,1 );
	hlOpClassAddImage(	c, "up_image",	"the up image", 1);

	/* -------------- DRAWING OPERATION ----------- */

	c = hlNewOpClass(	"draw_rect",	"draw a rectangle",
				HL_DRAW_RECT,	HL_DRAW,	NULL	);
	hlOpClassAddNum(	c,"pos_tl", "top left corner position",2,0,1);
	hlOpClassAddNum(	c,"pos_br", "bottom right corner position",2,0,1);
	hlOpClassAddNum(	c,"alpha", "the opacity of the drawing",1,0,1);
	hlOpClassAddColor(	c,"fill_color", "the fill color ",1);
	hlOpClassAddBBoxFun(	c, hlSquareBBoxFun);
	
	c = hlNewOpClass(	"draw_circle",	"draw a circle",
				HL_DRAW_CIRCLE,	HL_DRAW,	NULL	);
	hlOpClassAddNum(	c,"pos_center","center of the circle",2,0,1);
	hlOpClassAddNum(	c,"radius_in",	"inner radius",1,0,500);
	hlOpClassAddNum(	c,"radius_out",	"outer radius"	,1,0,500);
	hlOpClassAddNum(	c,"alpha",	"opacity of the circle",1,0,1);
	hlOpClassAddColor(	c,"fill_color", "the fill color ",1);
	hlOpClassAddBBoxFun(	c, hlCircleBBoxFun);

	c = hlNewOpClass(	"draw_triangle",	"draw a triangle",
				HL_DRAW_TRIANGLE,	HL_DRAW,	NULL	);
	hlOpClassAddNum(	c,"v0","coordinates of the first vertex",2,0,1);
	hlOpClassAddNum(	c,"v1","coordinates of the second vertex",2,0,1);
	hlOpClassAddNum(	c,"v2","coordinates of the third vertex",2,0,1);
	hlOpClassAddNum(	c,"alpha",	"opacity of the triangle",1,0,1);
	hlOpClassAddColor(	c,"fill_color", "the fill color ",1);
	hlOpClassAddBBoxFun(	c, hlTriangleBBoxFun);



}

