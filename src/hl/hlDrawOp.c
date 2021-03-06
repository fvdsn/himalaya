#include <math.h>
#include <stdio.h>
#include <string.h>
#include "hlDrawOp.h"
#include "hlBlendOp.h"

float   gauss_val[] = { 1.0,0.993237800913,0.973224332894,0.940760679767,0.89712267787,0.843977740268,0.783279263113,0.717147923098,0.647749878838,0.577181516904,0.507369047778,0.439989186411,0.376414648433,0.317685584388,0.264505666312,0.217259577768,0.176047297128,0.140729865175,0.110981262484,0.0863414881196,0.0662667814561,0.0501739976457,0.0374772755567,0.0276161923006,0.0200754858175,0.0143970929592,0.0101856801229,0.00710905275402,0.00489485600228,0.00332486933677,0.00222800276101,0.00147286637003,0.000960544405229};

int	gauss_valc = 32;

static float get_gauss(float dist, float max_dist){
	float factor = (dist/max_dist)*gauss_valc;
	int   index  = (int)factor;
	float alpha  = factor - index;
	if (index <= 0){
		return 1.0;
	}else if(index >=gauss_valc){
		return 0.0;
	}else{
		return gauss_val[index]*(1-alpha) + gauss_val[index+1]*(alpha);
	}
}

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
static void hl_draw_rect_8b(hlTile *a,hlColor *c, int chan,float *num, int tx, int ty, int tz){
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

static int hl_draw_circle_8b(hlTile *a,hlColor *c, int chan,float *num, int tx, int ty, int tz){
	uint8_t* data = HL_DATA_8B(a);
	const uint8_t* color = hlColorGetData(c);
	int X = tx*HL_TILEWIDTH;
	int Y = ty*HL_TILEWIDTH;
	float cx = (int)hl_scale_float(num[0],tz);
	float cy = (int)hl_scale_float(num[1],tz);
	float r0 = hl_scale_float(num[2],tz);
	float r1 = hl_scale_float(num[3],tz);
	int x = HL_TILEWIDTH;
	int y = HL_TILEWIDTH;
	float r,xf,yf;
	float f = 0;
	int oversampling = 0;
	#define PIXR 0.56418f
	if(r1 < 3*PIXR){
		oversampling = 1;
	}
	if(r0 > PIXR && r1-r0 <= 2*PIXR){
		float c = (r1+r0)/2.0f;
		r0 = c - PIXR;
		r1 = c + 2*PIXR;
	}
	r0 = powf(r0,2.0f);
	r1 = powf(r1,2.0f);
	if(X > cx +r1 || Y > cy +r1 || X+x < cx-r1 || Y+y < cy-r1){
		return 0;
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
			yf = y; //+ 0.5f;
			x = HL_TILEWIDTH;
			while(x--){
				xf = x;// + 0.5f;
				r = (X+xf)*(X+xf)+(Y+yf)*(Y+yf);
				if(r > r1+PIXR){
					continue;
				}else if (r + PIXR < r0){
					f = num[4];
				}else {
					f = get_gauss(r-r0,r1-r0)*num[4];
					/*	if(r - PIXR < r0){
						float fg = (r0 - r - PIXR)/(2*PIXR);
						f = fg + f*(1-fg);
					}*/
					//f = (float)(r1-r)/(float)(r1-r0)*num[4];
				}
				if(r1 < PIXR){
					f *= r1*r1*3.141592;
					if (f < 1.0f / 255.0f){
						if (f < 1.0f / 1024.0f){
							continue;
						}else{
							f = 1.0f / 255.0f;
						}
					}
				}
				hl_blend_mix_8b(data + (y*HL_TILEWIDTH+x)*chan,
						color,
						chan,
						f);
			}
		}
	}
	return oversampling;
}

void hlTriangleBBoxFun(const hlOp *op, hlBBox *box){
	float minx = fminf(op->p_num[0], fminf(op->p_num[2], op->p_num[4]));
	float maxx = fmaxf(op->p_num[0], fmaxf(op->p_num[2], op->p_num[4]));
	float miny = fminf(op->p_num[1], fminf(op->p_num[3], op->p_num[5]));
	float maxy = fmaxf(op->p_num[1], fmaxf(op->p_num[3], op->p_num[5]));

	box->infinite = 0;
	box->tx  = (int)(floorf(minx/HL_TILEWIDTH));
	box->ty  = (int)(floorf(miny/HL_TILEWIDTH));
	box->btx = (int)(floorf(maxx/HL_TILEWIDTH)) + 1;
	box->bty = (int)(floorf(maxy/HL_TILEWIDTH)) + 1;
}
static int hl_draw_tri_8b(hlTile *a, hlColor *c, int chan, float *num, int tx, int ty, int tz){
	uint8_t* data = HL_DATA_8B(a);
	const uint8_t* color = hlColorGetData(c);
	float X = tx*HL_TILEWIDTH;
	float Y = ty*HL_TILEWIDTH;
	float x0 = hl_scale_float(num[0],tz) - X;
	float y0 = hl_scale_float(num[1],tz) - Y;
	float x1 = hl_scale_float(num[2],tz) - X;
	float y1 = hl_scale_float(num[3],tz) - Y;
	float x2 = hl_scale_float(num[4],tz) - X;
	float y2 = hl_scale_float(num[5],tz) - Y;
	float maxx = fmaxf(x0,fmaxf(x1,x2));
	float maxy = fmaxf(y0,fmaxf(y1,y2));
	float minx = fminf(x0,fminf(x1,x2));
	float miny = fminf(y0,fminf(y1,y2));
	float dx01 = x0 - x1;
	float dx12 = x1 - x2;
	float dx20 = x2 - x0;
	float dy01 = y0 - y1;
	float dy12 = y1 - y2;
	float dy20 = y2 - y0;
	int x = HL_TILEWIDTH;
	int y = HL_TILEWIDTH;
	int oversampling = 0;
	if(	minx >= HL_TILEWIDTH || miny >= HL_TILEWIDTH || maxx < 0.0f || maxy < 0.0f ){
		return 0;
	}
	if(     (dx01 * (0.0f - y0) - dy01 * (0.0f - x0)) < 0.0f &&
		(dx12 * (0.0f - y1) - dy12 * (0.0f - x1)) < 0.0f &&
		(dx20 * (0.0f - y2) - dy20 * (0.0f - x2)) < 0.0f && 
		(dx01 * (HL_TILEWIDTH - y0) - dy01 * (0.0f - x0)) < 0.0f &&
		(dx12 * (HL_TILEWIDTH - y1) - dy12 * (0.0f - x1)) < 0.0f &&
		(dx20 * (HL_TILEWIDTH - y2) - dy20 * (0.0f - x2)) < 0.0f && 
		(dx01 * (HL_TILEWIDTH - y0) - dy01 * (HL_TILEWIDTH - x0)) < 0.0f &&
		(dx12 * (HL_TILEWIDTH - y1) - dy12 * (HL_TILEWIDTH - x1)) < 0.0f &&
		(dx20 * (HL_TILEWIDTH - y2) - dy20 * (HL_TILEWIDTH - x2)) < 0.0f && 
		(dx01 * (0.0f - y0) - dy01 * (HL_TILEWIDTH - x0)) < 0.0f &&
		(dx12 * (0.0f - y1) - dy12 * (HL_TILEWIDTH - x1)) < 0.0f &&
		(dx20 * (0.0f - y2) - dy20 * (HL_TILEWIDTH - x2)) < 0.0f ){
		while(x--){
			y = HL_TILEWIDTH;
			while(y--){
				hl_blend_mix_8b(data + (y*HL_TILEWIDTH+x)*chan,
						color,
						chan,
						num[6]);
			}
		}
	}else{	
		x = (int)minx < 0 ? 0 : (int)minx;
		while(x < HL_TILEWIDTH && x <= maxx){
			y = (int)miny < 0 ? 0 : (int)miny;
			while(y < HL_TILEWIDTH && y <= maxy){
				/*float f0,f1,f2,f;
				f = 1.0f;
				f0 = ( (dx01 * (y - y0)) - (dy01 * (x - x0)) );
				if(f0 > 10.0f){
					y++;
					continue;
				}else if( f0 >= -10.0f){
					f *= (10.0f+f0)/20.0f;
				}
				f1 = (dx12 * (y - y1)) - (dy12 * (x - x1));
				if(f1 > 10.0f){
					y++;
					continue;
				}else if( f1 >= -10.0f){
					f *= (10.0f+f1)/20.0f;
				}
				f2 = (dx20 * (y - y2)) - (dy20 * (x - x2));
				if(f2 > 10.0f){
					y++;
					continue;
				}else if (f2 >= -10.0f){
					f *= (10.0f+f2)/20.0f;
				}
				hl_blend_mix_8b(data + (y*HL_TILEWIDTH+x)*chan,
							color,
							chan,
							num[6]*f);
				y++;*/
				if(  ( (dx01 * (y - y0)) - (dy01 * (x - x0)) ) < 0.0f &&
				     ( (dx12 * (y - y1)) - (dy12 * (x - x1)) ) < 0.0f &&
				     ( (dx20 * (y - y2)) - (dy20 * (x - x2)) ) < 0.0f ){
					hl_blend_mix_8b(data + (y*HL_TILEWIDTH+x)*chan,
							color,
							chan,
							num[6]);
				}
				y++;
			}
			x++;
		}
	}
	return oversampling;
}
/*
static int hl_draw_line_8b(hlTile *a, hlColor *c, int chan, float *num, int tx, int ty, int tz){
	uint8_t* data = HL_DATA_8B(a);
	const uint8_t* color = hlColorGetData(c);
	float X = tx*HL_TILEWIDTH;
	float Y = ty*HL_TILEWIDTH;
	float x0 = hl_scale_float(num[0],tz) - X;
	float y0 = hl_scale_float(num[1],tz) - Y;
	float x1 = hl_scale_float(num[2],tz) - X;
	float y1 = hl_scale_float(num[3],tz) - Y;
	float d  = hl_scale_float(num[4],tz);
	float maxx = fmaxf(x0,x1);
	float maxy = fmaxf(y0,y1);
	float minx = fminf(x0,x1);
	float miny = fminf(y0,y1);
	float dx01 = x0 - x1;
	float dx12 = x1 - x2;
	float dy01 = y0 - y1;
	float dy12 = y1 - y2;
	int x = HL_TILEWIDTH;
	int y = HL_TILEWIDTH;
	int oversampling = 0;
	return oversampling;
}*/

static int hl_draw_8b(hlTile *a, int id, hlColor *c, int chan, float *num, int tx, int ty, int tz){
	switch(id){
		case HL_DRAW_RECT:
		hl_draw_rect_8b(a,c,chan,num,tx,ty,tz);
		case HL_DRAW_CIRCLE:
		return hl_draw_circle_8b(a,c,chan,num,tx,ty,tz);
		case HL_DRAW_TRIANGLE:
		return hl_draw_tri_8b(a,c,chan,num,tx,ty,tz);
	}
	return 0;
}

int hlDrawOp(hlTile *a, hlOp *p, int tx, int ty, int tz){
	int id = hlOpGetId(p);
	hlColor *c = hlOpGetAllColor(p);
	hlCS cs = hlColorGetCS(c);
	int chan = hlCSGetChan(cs);
	float *num = hlOpGetAllValue(p);
	switch(hlCSGetBpc(cs)){
		case HL_8B:
		return hl_draw_8b(a,id,c,chan,num,tx,ty,tz);
		break;
		case HL_32B:
		default: 
		return 0;

	}
}

