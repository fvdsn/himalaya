#include <stdint.h> 
#include <math.h>
#include "hlBlendOp.h"

/*32 bits floating point blending*/
static float hl_b_mix_32b(float down, float up, float alpha){
	return down*(1.0-alpha)+up*alpha;
}
static float hl_b_add_32b(float down, float up, float alpha){
	return down + up*alpha;
}
static float hl_b_mult_32b(float down, float up, float alpha){
	return hl_b_mix_32b(down,up*down,alpha);
}
static void hl_blend_mix_32b(float *down, const float *up, int channels, float mix){
	float alphaup   = up[channels-1]*mix;
	float alphadown = down[channels-1];
	if(alphaup > 1.0 || alphaup < 0.0)
		return;
	switch(channels){
		case 5:
		down[3] = hl_b_mix_32b(down[3],up[3],alphaup);
		case 4:
		down[2] = hl_b_mix_32b(down[2],up[2],alphaup);
		case 3:
		down[1] = hl_b_mix_32b(down[1],up[1],alphaup);
		case 2:
		down[0] = hl_b_mix_32b(down[0],up[0],alphaup);
		case 1:
		down[channels-1] = alphadown + (1.0 - alphadown) * alphaup;
	}
}
static void hl_blend_add_32b(float *down, const float *up, int channels, float mix){
	float alphaup   = up[channels-1]*mix;
	float alphadown = down[channels-1];
	switch(channels){
		case 5:
		down[3] = hl_b_add_32b(down[3],up[3],alphaup);
		case 4:
		down[2] = hl_b_add_32b(down[2],up[2],alphaup);
		case 3:
		down[1] = hl_b_add_32b(down[1],up[1],alphaup);
		case 2:
		down[0] = hl_b_add_32b(down[0],up[0],alphaup);
		case 1:
		down[channels-1] = alphadown + (1.0 - alphadown) * alphaup;
	}
}
static void hl_blend_mult_32b(float *down, const float *up, int channels, float mix){
	float alphaup   = up[channels-1]*mix;
	float alphadown = down[channels-1];
	switch(channels){
		case 5:
		down[3] = hl_b_mult_32b(down[3],up[3],alphaup);
		case 4:
		down[2] = hl_b_mult_32b(down[2],up[2],alphaup);
		case 3:
		down[1] = hl_b_mult_32b(down[1],up[1],alphaup);
		case 2:
		down[0] = hl_b_mult_32b(down[0],up[0],alphaup);
		case 1:
		down[channels-1] = alphadown + (1.0 - alphadown) * alphaup;
	}
}

/*8 bits integer blending*/
static uint8_t hl_b_mix_8b(uint8_t down, uint8_t up, uint8_t alpha){
	return (down*(255-alpha)+up*alpha)/255;
}
static uint8_t hl_b_add_8b(uint8_t down, uint8_t up, uint8_t alpha){
	uint8_t tmp = up*alpha/255 + down;
	if(tmp < down){
		return 255;
	}else{
		return tmp;
	}
}
static uint8_t hl_b_mult_8b(uint8_t down, uint8_t up, uint8_t alpha){
	return hl_b_mix_8b(down,down*up/255,alpha);
}
void hl_blend_mix_8b(uint8_t *down, const uint8_t*up, int channels, float mix){
	uint8_t alphaup   = up[channels-1]*mix;
	uint8_t alphadown = down[channels-1];
	switch(channels){
		case 5:
		down[3] = hl_b_mix_8b(down[3],up[3],alphaup);
		case 4:
		down[2] = hl_b_mix_8b(down[2],up[2],alphaup);
		case 3:
		down[1] = hl_b_mix_8b(down[1],up[1],alphaup);
		case 2:
		down[0] = hl_b_mix_8b(down[0],up[0],alphaup);
		case 1:
		down[channels-1] = alphadown + ((255 - alphadown) * alphaup)/255;
	}
}
static void hl_blend_add_8b(uint8_t *down, const uint8_t*up, int channels, float mix){
	uint8_t alphaup   = up[channels-1]*mix;
	uint8_t alphadown = down[channels-1];
	switch(channels){
		case 5:
		down[3] = hl_b_add_8b(down[3],up[3],alphaup);
		case 4:
		down[2] = hl_b_add_8b(down[2],up[2],alphaup);
		case 3:
		down[1] = hl_b_add_8b(down[1],up[1],alphaup);
		case 2:
		down[0] = hl_b_add_8b(down[0],up[0],alphaup);
		case 1:
		down[channels-1] = alphadown + ((255 - alphadown) * alphaup)/255;
	}
}
static void hl_blend_mult_8b(uint8_t *down, const uint8_t*up, int channels, float mix){
	uint8_t alphaup   = up[channels-1]*mix;
	uint8_t alphadown = down[channels-1];
	switch(channels){
		case 5:
		down[3] = hl_b_mult_8b(down[3],up[3],alphaup);
		case 4:
		down[2] = hl_b_mult_8b(down[2],up[2],alphaup);
		case 3:
		down[1] = hl_b_mult_8b(down[1],up[1],alphaup);
		case 2:
		down[0] = hl_b_mult_8b(down[0],up[0],alphaup);
		case 1:
		down[channels-1] = alphadown + ((255 - alphadown) * alphaup)/255;
	}
}

static void hl_blend_8b(hlTile*a, const hlTile*b, int id, int chan, float *num){
	int i = chan*HL_TILEWIDTH*HL_TILEWIDTH;
	uint8_t* A = HL_DATA_8B(a);
	const uint8_t* B = HL_DATA_8B(b);
	while((i-=chan) >= 0){
		switch(id){
			case HL_BLEND_MIX:
				hl_blend_mix_8b(A+i,B+i,chan,num[0]);
				break;
			case HL_BLEND_ADD:
				hl_blend_add_8b(A+i,B+i,chan,num[0]);
				break;
			case HL_BLEND_MULT:
				hl_blend_mult_8b(A+i,B+i,chan,num[0]);
				break;

		}
		
	}
	
}
static void hl_blend_32b(hlTile*a, const hlTile*b, int id, int chan, float *num){
	int i = chan*HL_TILEWIDTH*HL_TILEWIDTH;
	float* A = HL_DATA_32B(a);
	const float* B = HL_DATA_32B(b);
	while((i-=chan) >= 0){
		switch(id){
			case HL_BLEND_MIX:
				hl_blend_mix_32b(A+i,B+i,chan,num[0]);
				break;
			case HL_BLEND_ADD:
				hl_blend_add_32b(A+i,B+i,chan,num[0]);
				break;
			case HL_BLEND_MULT:
				hl_blend_mult_32b(A+i,B+i,chan,num[0]);
				break;

		}
		
	}
	
}
void hlBlendOp(hlTile *a,const hlTile *b, hlOp *op){
	int id = hlOpGetId(op);
	int chan = hlCSGetChan(hlOpGetCSIn(op));
	float *num = hlOpGetAllValue(op);
	switch(hlCSGetBpc(hlOpGetCSIn(op))){
		case HL_8B:
			hl_blend_8b(a,b,id,chan,num);
			break;
		case HL_32B:
			hl_blend_32b(a,b,id,chan,num);
			break;
	}
}

	

