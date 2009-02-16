#include <math.h>
#include <stdio.h>
#include <string.h>
#include "hlDrawOp.h"
#include "hlBlendOp.h"


static float hl_scale_float(float dist, int tz){
	while(tz--){
		dist /= 2;
	}
	return dist;
}
static void hl_draw_rect_8b(hlTile *a,hlColor *c, int chan,float *num, int tx, int ty, unsigned int tz){
	uint8_t* data = HL_DATA_8B(a);
	const uint8_t *color = hlColorGetData(c);
	const int X = tx*HL_TILEWIDTH;
	const int Y = ty*HL_TILEWIDTH;
	const int topx = (int)hl_scale_float(num[0],tz);
	const int topy = (int)hl_scale_float(num[1],tz);
	const int botx = (int)hl_scale_float(num[2],tz);
	const int boty = (int)hl_scale_float(num[3],tz);
	const int bpp  = chan;
	int x = HL_TILEWIDTH;
	int y = HL_TILEWIDTH;
	if(X > botx || Y > boty || X+x <topx || Y+y < topy){return;}
	while(y--){
		if(Y+y < topy){
			break;
		}else if(Y+y >= boty){
			continue;
		}
		x = HL_TILEWIDTH;
		while(x--){
			if(X+x < topx){
				break;
			}else if(X+x >=botx){
				continue;
			}
			hl_blend_mix_8b(data + (y*HL_TILEWIDTH+x)*bpp,
					color,
					chan,
					num[4]);
		}
	}
}
static void hl_draw_circle_8b(hlTile *a,hlColor *c, int chan,float *num, int tx, int ty, unsigned int tz){
	uint8_t* data = HL_DATA_8B(a);
	const uint8_t* color = hlColorGetData(c);
	int X = tx*HL_TILEWIDTH;
	int Y = ty*HL_TILEWIDTH;
	int cx = (int)hl_scale_float(num[0],tz);
	int cy = (int)hl_scale_float(num[1],tz);
	int r0 = pow(hl_scale_float(num[2],tz),2);
	int r1 = pow(hl_scale_float(num[3],tz),2);
	int x = HL_TILEWIDTH;
	int y = HL_TILEWIDTH;
	int r;
	float f = 0;
	if(X > cx +r1 || Y > cy +r1 || X+x < cx-r1 || X+y < cy-r1){
		return;
	}else if( 	( (X-cx)*(X-cx)+(Y-cy)*(Y-cy) < r0 ) &&
			( (X-cx+x)*(X-cx+x)+(Y-cy)*(Y-cy) < r0 ) &&
			( (X-cx+x)*(X-cx+x)+(Y-cy+y)*(Y-cy+y) < r0 ) &&
			( (X-cx)*(X-cx)+(Y-cy+y)*(Y-cy+y) < r0 ) ){
		x = HL_TILEWIDTH*HL_TILEWIDTH*chan;
		do{
			hl_blend_mix_8b(data + x, color, chan, num[4]);
		}while((x-=chan)>=0);
	}else{
		X-=cx;
		Y-=cy;
		while(y--){
			x = HL_TILEWIDTH;
			while(x--){
				r = (X+x)*(X+x)+(Y+y)*(Y+y);
				if(r > r1){
					continue;
				}else if (r < r0){
					f = num[4];
				}else{
					f = (float)(r1-r)/(float)(r1-r0)*num[4];
				}
				hl_blend_mix_8b(data + (y*HL_TILEWIDTH+x)*chan,
						color,
						chan,
						f);
			}
		}
	}
}

static void hl_draw_8b(hlTile *a, int id, hlColor *c, int chan, float *num, int tx, int ty, unsigned int tz){
	switch(id){
		case HL_DRAW_RECT:
		hl_draw_rect_8b(a,c,chan,num,tx,ty,tz);
		break;
		case HL_DRAW_CIRCLE:
		hl_draw_circle_8b(a,c,chan,num,tx,ty,tz);
		break;
	}
}

void hlDrawOp(hlTile *a, hlOp *p, int tx, int ty, unsigned int tz){
	int id = hlOpGetId(p);
	hlColor *c = hlOpGetAllColor(p);
	hlCS cs = hlColorGetCS(c);
	int chan = hlCSGetChan(cs);
	float *num = hlOpGetAllValue(p);
	switch(hlCSGetBpc(cs)){
		case HL_8B:
		hl_draw_8b(a,id,c,chan,num,tx,ty,tz);
		break;
		default: 
		return;

	}
}

