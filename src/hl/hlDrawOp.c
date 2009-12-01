#include <math.h>
#include <stdio.h>
#include <string.h>
#include "hlDrawOp.h"
#include "hlBlendOp.h"


static float hl_scale_float(float dist, int tz){
	return dist*powf(2,-tz);
}
void hlSquareBBoxFun(const hlOp *op, hlBBox *box){
	box->infinite = 0;
	box->tx = (int)(floorf(op->p_num[0]/HL_TILEWIDTH));
	box->ty = (int)(floorf(op->p_num[1]/HL_TILEWIDTH));
	box->btx = (int)(floorf(op->p_num[2]/HL_TILEWIDTH)) + 1;
	box->bty = (int)(floorf(op->p_num[3]/HL_TILEWIDTH)) + 1;
	//printf("BBox : Rect : [%d,%d | %d,%d]\n",box->tx,box->ty,box->btx,box->bty);
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
void hlCircleBBoxFun(const hlOp *op, hlBBox *box){
	float cx = op->p_num[0];
	float cy = op->p_num[1];
	float r = op->p_num[3];
	box->infinite = 0;
	box->tx  = (int)(floorf((cx-r)/HL_TILEWIDTH));
	box->ty  = (int)(floorf((cy-r)/HL_TILEWIDTH));
	box->btx = (int)(floorf((cx+r)/HL_TILEWIDTH)) + 1;
	box->bty = (int)(floorf((cy+r)/HL_TILEWIDTH)) + 1;
	//printf("BBox : Circle : [%d,%d | %d,%d]\n",box->tx,box->ty,box->btx,box->bty);
}

static void hl_draw_circle_8b(hlTile *a,hlColor *c, int chan,float *num, int tx, int ty, unsigned int tz){
	uint8_t* data = HL_DATA_8B(a);
	const uint8_t* color = hlColorGetData(c);
	int X = tx*HL_TILEWIDTH;
	int Y = ty*HL_TILEWIDTH;
	float cx = (int)hl_scale_float(num[0],tz);
	float cy = (int)hl_scale_float(num[1],tz);
	float r0 = pow(hl_scale_float(num[2],tz),2);
	float r1 = pow(hl_scale_float(num[3],tz),2);
	int x = HL_TILEWIDTH;
	int y = HL_TILEWIDTH;
	float r;
	float f = 0;
	if(X > cx +r1 || Y > cy +r1 || X+x < cx-r1 || Y+y < cy-r1){
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
				if(r1 < 0.5f){
					f *= 2*r1;
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
		case HL_32B:
		default: 
		return;

	}
}
void hlDrawVec(hlTile *a, hlOp *p, int opindex, int tx, int ty, unsigned int tz){
	int id = hlOpGetId(p);
	hlColor *c = hlOpGetAllColor(p);
	hlCS cs = hlColorGetCS(c);
	int chan = hlCSGetChan(cs);
	float *num = p->vector->p_num_vec[opindex];
	if(opindex <0 || opindex >= p->vector->opcount){
		printf("WARNING: opindex out of bounds : index%d, opcount:%d\n",
				opindex,p->vector->opcount);
	}
	switch(hlCSGetBpc(cs)){
		case HL_8B:
		hl_draw_8b(a,id,c,chan,num,tx,ty,tz);
		break;
		case HL_32B:
		default: 
		return;

	}
}

