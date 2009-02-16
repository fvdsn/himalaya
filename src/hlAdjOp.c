#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include "hlAdjOp.h"

#define MAX_FLOAT 1000000

/* 	hlAdjOpSkip(...) 		*/
void hlAdjOpSkip(hlTile *t, hlOp *p){
	t = NULL;	/*stupid warnings*/
	p = NULL;
	return;
}

/* 	hlAdjOpInvert(...) 		*/
void hl_invert_8b(hlTile *t, const uint32_t chan){
	int32_t i = chan*HL_TILEWIDTH*HL_TILEWIDTH;
	int32_t j = chan - 1;
	uint8_t * data = HL_DATA_8B(t);
	while((i-=chan)>= 0){
		j = chan - 1;
		while(j--){
			data[i+j] = 255 - data[i+j];
		}
	}
}
void hl_invert_32b(hlTile *t, const uint32_t chan){
	int32_t i = chan* HL_TILEWIDTH*HL_TILEWIDTH;
	int32_t j = chan - 1;
	float * data = HL_DATA_32B(t);
	while((i-=chan)>=0){
		j = chan - 1;
		while (j--){
			data[i+j] = 1.0 - data[i+j];
		}
	}
}
void hlAdjOpInvert(hlTile *t, hlOp *p){
	uint32_t chan = hlCSGetChan(hlOpGetCSIn(p));
	switch(hlCSGetBpc(hlOpGetCSIn(p))){
		case HL_8B:{hl_invert_8b(t,chan);break;}
		case HL_32B:{hl_invert_32b(t,chan);break;}
	}
}

/* 	hlAdjOpFill(...) 		*/
void hlAdjOpFill(hlTile *t, hlOp *p){
	hlTileFill(t,hlOpGetAllColor(p));
}

/* 	hlAdjOpMath(...) 		*/
uint8_t hl_add_8b( uint8_t a, int i){
	int tmp = a + i;
	if (tmp > 255)
		return 255;
	else if (tmp < 0)
		return 0;
	else
		return (uint8_t)tmp;
}
uint8_t hl_mul_8b(uint8_t a, float b){
	int tmp = a*b;
	if (tmp > 255)
		return 255;
	else if (tmp < 0)
		return 0;
	return (uint8_t) tmp;
}
uint8_t hl_div_8b(uint8_t a, float b){
	int tmp;
	if ( b == 0.0)
		return 255;
	else if ( (tmp = a/b) > 255 )
		return 255;
	else if (tmp < 0)
		return 0;
	return (uint8_t) tmp;
}
uint8_t hl_more_8b(uint8_t a, uint8_t b){
	if (a > b){
		return a;
	}else{
		return b;
	}
}
uint8_t hl_less_8b(uint8_t a, uint8_t b){
	if (a < b){
		return a;
	}else{
		return b;
	}
}
uint8_t hl_pow_8b(uint8_t a, float b){
	float fa = (float)a / 255.0;
	fa = pow(fa,b);
	return (uint8_t)(fa*255.0);
}
void hl_math_8b(hlTile *t, unsigned int id, const uint32_t chan, const float * f){
	int32_t i = chan*HL_TILEWIDTH*HL_TILEWIDTH;
	uint8_t * data = HL_DATA_8B(t);
	int  fact[5];
	uint8_t fact8[5];
	fact[0] = f[0]; 
	fact[1] = f[1];
	fact[2] = f[2];
	fact[3] = f[3];
	fact[4] = f[4];
	
	fact8[0] =(uint8_t) f[0]; 
	fact8[1] =(uint8_t) f[1];
	fact8[2] =(uint8_t) f[2];
	fact8[3] =(uint8_t) f[3];
	fact8[4] =(uint8_t) f[4];
	
	while((i-=chan)>= 0){
		switch(id){
		case HL_ADJ_ADD:{
			switch(chan){
				case 5 : data[i+3] = hl_add_8b(data[i+3], fact[3]);
				case 4 : data[i+2] = hl_add_8b(data[i+2], fact[2]);
				case 3 : data[i+1] = hl_add_8b(data[i+1], fact[1]);
				case 2 : data[i]   = hl_add_8b(data[i],   fact[0]);
				case 1 : { 
					data[i+ chan - 1] = 
						hl_add_8b(data[i+chan-1], fact[4]);
					break;
				}
			}
			break;
		}
		case HL_ADJ_MUL:{
			switch(chan){
				case 5 : data[i+3] = hl_mul_8b(data[i+3], f[3]);
				case 4 : data[i+2] = hl_mul_8b(data[i+2], f[2]);
				case 3 : data[i+1] = hl_mul_8b(data[i+1], f[1]);
				case 2 : data[i]   = hl_mul_8b(data[i], f[0]);
				case 1 : { 
					data[i+ chan - 1] = 
						hl_mul_8b(data[i+chan-1],f[4]);
					break;
				}
			}
			break;
		}
		case HL_ADJ_DIV:{
			switch(chan){
				case 5 : data[i+3] = hl_div_8b(data[i+3], f[3]);
				case 4 : data[i+2] = hl_div_8b(data[i+2], f[2]);
				case 3 : data[i+1] = hl_div_8b(data[i+1], f[1]);
				case 2 : data[i]   = hl_div_8b(data[i], f[0]);
				case 1 : { 
					data[i+ chan - 1] = 
						hl_div_8b(data[i+chan-1],f[4]);
					break;
				}
			}
			break;
		}
		case HL_ADJ_MOD:{
			switch(chan){ 
				case 5 : data[i+3] = fact[3] == 0 ? 0 
							: data[i+3] % fact[3];
				case 4 : data[i+2] = fact[2] == 0 ? 0
							: data[i+2] % fact[2];
				case 3 : data[i+1] = fact[1] == 0 ? 0
							: data[i+1] % fact[1];
				case 2 : data[i]   = fact[0] == 0 ? 0
							: data[i] % fact[0];
				case 1 : { 
					data[i+ chan - 1] = fact[4] == 0 ? 0
						: data[i+chan-1] % fact[4];
					break;
				}
			}
			break;
		}
		case HL_ADJ_LESS:{
			switch(chan){
				case 5 : data[i+3] = hl_less_8b(data[i+3], fact8[3]);
				case 4 : data[i+2] = hl_less_8b(data[i+2], fact8[2]);
				case 3 : data[i+1] = hl_less_8b(data[i+1], fact8[1]);
				case 2 : data[i]   = hl_less_8b(data[i],   fact8[0]);
				case 1 : { 
					data[i+ chan - 1] = 
						hl_less_8b(data[i+chan-1], fact8[4]);
					break;
				}
			}
			break;
		}
		case HL_ADJ_MORE:{
			switch(chan){
				case 5 : data[i+3] = hl_more_8b(data[i+3], fact8[3]);
				case 4 : data[i+2] = hl_more_8b(data[i+2], fact8[2]);
				case 3 : data[i+1] = hl_more_8b(data[i+1], fact8[1]);
				case 2 : data[i]   = hl_more_8b(data[i],   fact8[0]);
				case 1 : { 
					data[i+ chan - 1] = 
						hl_more_8b(data[i+chan-1], fact8[4]);
					break;
				}
			}
			break;
		}
		case HL_ADJ_POW:{
			switch(chan){
				case 5 : data[i+3] = hl_pow_8b(data[i+3], f[3]);
				case 4 : data[i+2] = hl_pow_8b(data[i+2], f[2]);
				case 3 : data[i+1] = hl_pow_8b(data[i+1], f[1]);
				case 2 : data[i]   = hl_pow_8b(data[i], f[0]);
				case 1 : { 
					data[i+ chan - 1] = 
						hl_pow_8b(data[i+chan-1],f[4]);
					break;
				}
			}
			break;
		}
		}
	}
	return;
}

void hl_math_32b(hlTile *t, unsigned int id, const uint32_t chan, const float * f){
	int32_t i = chan*HL_TILEWIDTH*HL_TILEWIDTH;
	float * data = HL_DATA_32B(t);
	while((i-=chan)>= 0){
		switch(id){
		case HL_ADJ_ADD:{
			switch(chan){
				case 5 : data[i+3] += f[3]; 
				case 4 : data[i+2] += f[2];
				case 3 : data[i+1] += f[1];
				case 2 : data[i]   += f[0];
				case 1 : { 
					data[i+ chan - 1] += f[4]; 
					break;
				}
			}
			break;
		}
		case HL_ADJ_MUL:{
			switch(chan){
				case 5 : data[i+3] *= f[3]; 
				case 4 : data[i+2] *= f[2];
				case 3 : data[i+1] *= f[1];
				case 2 : data[i]   *= f[0];
				case 1 : { 
					data[i+ chan - 1] *= f[4]; 
					break;
				}
			}
			break;
		}
		case HL_ADJ_DIV:{
			switch(chan){
				case 5 : data[i+3] = f[3] == 0.0 ? MAX_FLOAT :
							data[i+3]/f[3];
				case 4 : data[i+2] = f[2] == 0.0 ? MAX_FLOAT :
							data[i+2]/f[2];
				case 3 : data[i+1] = f[1] == 0.0 ? MAX_FLOAT :
							data[i+1]/f[1];
				case 2 : data[i]   = f[0] == 0.0 ? MAX_FLOAT :
							data[i]/f[0];
				case 1 : { 
					data[i+ chan - 1] = f[4] == 0.0 
						? MAX_FLOAT 
						: data[i + chan - 1] / f[4];
					break;
				}
			}
			break;
		}
		case HL_ADJ_MOD:{
			switch(chan){ /*doesn't work
				case 5 : data[i+3]%= (int)f[3];
				case 4 : data[i+2]%= (int)f[2];
				case 3 : data[i+1]%= (int)f[1];
				case 2 : data[i]  %= (int)f[0];
				case 1 : { 
					data[i+ chan - 1] %= (int)f[4];
					break;
				}*/
				default : break;
			}
			break;
		}
		case HL_ADJ_LESS:{
			switch(chan){
				case 5 : data[i+3]= data[i+3] < f[3] ?
						data[i+3]:
						f[3];
				case 4 : data[i+3]= data[i+2] < f[2] ?
						data[i+2]:
						f[2];
				case 3 : data[i+1]= data[i+1] < f[1] ?
						data[i+1]:
						f[1];
				case 2 : data[i]= data[i] < f[0] ?
						data[i]:
						f[0];
				case 1 : { 
					data[i+chan-1] = data[i+ chan - 1] < f[4] ?
						data[i+chan-1]:
						f[4]; 
					break;
				}
			}
			break;
		}
		case HL_ADJ_MORE:{
			switch(chan){
				case 5 : data[i+3]= data[i+3] > f[3] ?
						data[i+3]:
						f[3];
				case 4 : data[i+3]= data[i+2] > f[2] ?
						data[i+2]:
						f[2];
				case 3 : data[i+1]= data[i+1] > f[1] ?
						data[i+1]:
						f[1];
				case 2 : data[i]= data[i] > f[0] ?
						data[i]:
						f[0];
				case 1 : { 
					data[i+chan-1] = data[i+ chan - 1] > f[4] ?
						data[i+chan-1]:
						f[4]; 
					break;
				}
			}
			break;
		}
		case HL_ADJ_POW:{
			switch(chan){
				case 5 : data[i+3] = pow(data[i+3], f[3]); 
				case 4 : data[i+2] = pow(data[i+2],f[2]);
				case 3 : data[i+1] = pow(data[i+1],f[1]);
				case 2 : data[i]   = pow(data[i], f[0]);
				case 1 : { 
					data[i+ chan - 1] = pow(data[i+chan-1], f[4]); 
					break;
				}
			}
			break;
		}
		}
	}
	return;
}
void hlAdjOpMath(hlTile *t, hlOp *p){
	uint32_t chan = hlCSGetChan(hlOpGetCSIn(p));
	float *num = hlOpGetAllNum(p);
	int id = hlOpGetId(p);
	switch(hlCSGetBpc(hlOpGetCSIn(p))){
		case HL_8B:{  hl_math_8b(t,id,chan,num);break;}
		case HL_32B:{ /*hl_math_32b(t,p->id,chan,p->num)*/;break;}
	}
	return;
}

/* CHANNEL MIXER */
void hl_chanmix_8b(hlTile *t,unsigned int chan, const float *n){
	int32_t i = chan*HL_TILEWIDTH*HL_TILEWIDTH;
	uint8_t *d = HL_DATA_8B(t);
	while((i-=chan)>=0){
		switch (chan){
			case 5:
			d[i]   = n[0]*d[i]  +n[1]*d[i+1]  +n[2]*d[i+2]  +n[3]*d[i+3]  +n[4]*d[i+4];
			d[i+1] = n[5]*d[i]  +n[6]*d[i+1]  +n[7]*d[i+2]  +n[8]*d[i+3]  +n[9]*d[i+4];
			d[i+2] = n[10]*d[i] +n[11]*d[i+1] +n[12]*d[i+2] +n[13]*d[i+3] +n[14]*d[i+4];
			d[i+3] = n[15]*d[i] +n[16]*d[i+1] +n[17]*d[i+2] +n[18]*d[i+3] +n[19]*d[i+4];
			d[i+4] = n[20]*d[i] +n[21]*d[i+1] +n[22]*d[i+2] +n[23]*d[i+3] +n[24]*d[i+4];
			break;
			case 4:
			d[i]   = n[0]*d[i]  +n[1]*d[i+1]  +n[2]*d[i+2]  +n[3]*d[i+3];  
			d[i+1] = n[4]*d[i]  +n[5]*d[i+1]  +n[6]*d[i+2]  +n[7]*d[i+3];  
			d[i+2] = n[8]*d[i]  +n[9]*d[i+1]  +n[10]*d[i+2] +n[11]*d[i+3];  
			d[i+3] = n[12]*d[i] +n[13]*d[i+1] +n[14]*d[i+2] +n[15]*d[i+3];
			break;
			case 3:
			d[i]   = n[0]*d[i]  +n[1]*d[i+1]  +n[2]*d[i+2];  
			d[i+1] = n[3]*d[i]  +n[4]*d[i+1]  +n[5]*d[i+2];  
			d[i+2] = n[6]*d[i]  +n[7]*d[i+1]  +n[8]*d[i+2];  
			break;
			case 2:
			d[i]   = n[0]*d[i]  +n[1]*d[i+1];  
			d[i+1] = n[2]*d[i]  +n[3]*d[i+1];  
			break;
			default:
			return;
		}
	}
}
void hl_chanmix_32b(hlTile *t,unsigned int chan, const float *n){
	int32_t i = chan*HL_TILEWIDTH*HL_TILEWIDTH;
	float *d = HL_DATA_32B(t);
	while((i-=chan)>=0){
		switch (chan){
			case 5:
			d[i]   = n[0]*d[i]  +n[1]*d[i+1]  +n[2]*d[i+2]  +n[3]*d[i+3]  +n[4]*d[i+4];
			d[i+1] = n[5]*d[i]  +n[6]*d[i+1]  +n[7]*d[i+2]  +n[8]*d[i+3]  +n[9]*d[i+4];
			d[i+2] = n[10]*d[i] +n[11]*d[i+1] +n[12]*d[i+2] +n[13]*d[i+3] +n[14]*d[i+4];
			d[i+3] = n[15]*d[i] +n[16]*d[i+1] +n[17]*d[i+2] +n[18]*d[i+3] +n[19]*d[i+4];
			d[i+4] = n[20]*d[i] +n[21]*d[i+1] +n[22]*d[i+2] +n[23]*d[i+3] +n[24]*d[i+4];
			break;
			case 4:
			d[i]   = n[0]*d[i]  +n[1]*d[i+1]  +n[2]*d[i+2]  +n[3]*d[i+3];  
			d[i+1] = n[4]*d[i]  +n[5]*d[i+1]  +n[6]*d[i+2]  +n[7]*d[i+3];  
			d[i+2] = n[8]*d[i]  +n[9]*d[i+1]  +n[10]*d[i+2] +n[11]*d[i+3];  
			d[i+3] = n[12]*d[i] +n[13]*d[i+1] +n[14]*d[i+2] +n[15]*d[i+3];
			break;
			case 3:
			d[i]   = n[0]*d[i]  +n[1]*d[i+1]  +n[2]*d[i+2];  
			d[i+1] = n[3]*d[i]  +n[4]*d[i+1]  +n[5]*d[i+2];  
			d[i+2] = n[6]*d[i]  +n[7]*d[i+1]  +n[8]*d[i+2];  
			break;
			case 2:
			d[i]   = n[0]*d[i]  +n[1]*d[i+1];  
			d[i+1] = n[2]*d[i]  +n[3]*d[i+1];  
			break;
			default:
			return;
		}
	}
}
void hlAdjOpChanmix(hlTile *t, hlOp *p){
	uint32_t chan = hlCSGetChan(hlOpGetCSIn(p));
	float *num = hlOpGetAllNum(p);
	switch(hlCSGetBpc(hlOpGetCSIn(p))){
		case HL_8B:{ hl_chanmix_8b(t,chan,num);break;}
		case HL_32B:{ hl_chanmix_32b(t,chan,num);break;}
	}
	return;
}

