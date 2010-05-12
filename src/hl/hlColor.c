#include<math.h>
#include<string.h>
#include<stdio.h>
#include<assert.h>
#include<stdbool.h>
#include"hlColor.h"

/* 	hlNewColor(...) 		*/
hlColor hlNewColor(	hlCS cs, 
			float a, 
			float b, 
			float c, 
			float d, 
			float alpha ){
	hlColor col;
	col.cs = cs;
	switch(hlCSGetChan(cs)){
		case 1 : { 
			hlColorSetChan(&col,0,alpha);
			break;
		}
		case 2 : { 
			hlColorSetChan(&col,0,a);
			hlColorSetChan(&col,1,alpha);
			break;
		}
		case 3 : { 
			hlColorSetChan(&col,0,a);
			hlColorSetChan(&col,1,b);
			hlColorSetChan(&col,2,alpha);
			break;
		}
		case 4 : { 
			hlColorSetChan(&col,0,a);
			hlColorSetChan(&col,1,b);
			hlColorSetChan(&col,2,c);
			hlColorSetChan(&col,3,alpha);
			break;
		}
		case 5 : { 
			hlColorSetChan(&col,0,a);
			hlColorSetChan(&col,1,b);
			hlColorSetChan(&col,2,c);
			hlColorSetChan(&col,3,d);
			hlColorSetChan(&col,4,alpha);
			break;
		}
		default : { assert(0 && "wrong chan count"); break; }
			
	}
	return col;
}

/*	hlColorSet___(...) 		*/
void hlColorSetChan(hlColor* col, int channel, float value){
	float*    col32f = (float*)col->color;
	assert( channel >=0 && channel < hlCSGetChan(col->cs));
	switch(hlCSGetBpc(col->cs)){
		case HL_8B :
			col->color[channel] = (uint8_t)(value*255.0);
			break;
		case HL_32B : 
			col32f[channel] = value;
			break;
	}
	return;
}

/* 	hlColorGet___(...) 		*/
float hlColorGetChan(const hlColor* col, int channel){
	float*    col32f = (float*)col->color;
	assert( channel >= 0 && channel < hlCSGetChan(col->cs));
	switch(hlCSGetBpc(col->cs)){
		case HL_8B :	return (float)(col->color[channel]/255.0); 
		case HL_32B : return col32f[channel];
		default : { assert(0 && "wrong bpc"); return 42; }
	}
}
hlCS  hlColorGetCS(const hlColor* color){
	return color->cs;
}
void hlColorCopy(hlColor *dst, const hlColor *src){
	dst->cs = src->cs;
	memcpy((char*)dst->color,(char*)src->color,HL_MAX_BYTEPERPIXEL);
	return;
}
uint8_t *hlColorGetData(hlColor *color){
	return color->color;
}


void hlHslToRgb(float *hsl,float*rgb){
	float H = hsl[0]*360.0f;
	float C = hsl[2] <= 0.5f ? 2.0f * hsl[1]*hsl[2]:(2.0f - 2.0f*hsl[2])*hsl[1];
	float m = hsl[2] - C*0.5f;
	float Hp = H/60.0f;
	float HpMod2 = fmodf(Hp,2.0f); // Hp - 2.0f*( (int)(Hp/2.0f));
	float X = C*(1-fabs(HpMod2 - 1));
	if ( 0 <= Hp && Hp < 1){
		rgb[0] = C;
		rgb[1] = X;
		rgb[2] = 0;
	}else if( 1<= Hp && Hp < 2){
		rgb[0] = X;
		rgb[1] = C;
		rgb[2] = 0;
	}else if( 2<= Hp && Hp < 3){
		rgb[0] = 0;
		rgb[1] = C;
		rgb[2] = X;
	}else if( 3<= Hp && Hp < 4){
		rgb[0] = 0;
		rgb[1] = X;
		rgb[2] = C;
	}else if( 4<= Hp && Hp < 5){
		rgb[0] = X;
		rgb[1] = 0;
		rgb[2] = C;
	}else{
		rgb[0] = C;
		rgb[1] = 0;
		rgb[2] = X;
	}
	rgb[0] += m;
	rgb[1] += m;
	rgb[2] += m;
}
void hlRgbToHsl(float *rgb, float *hsl){
	float M,m,C,H,Hp,S,L;
	int i = 3;
	M = rgb[0];
	m = rgb[0];

	while(i--){
		if (rgb[i] > M){
			M = rgb[i];
		}
		if (rgb[i] < m){
			m = rgb[i];
		}
	}
	C = M - m;
	if (C == 0.0){
		Hp = 0.0;
	}else if(M == rgb[0]){
		Hp = fmodf((rgb[1] - rgb[2])/C,6.0f);
	}else if(M == rgb[1]){
		Hp = (rgb[2] - rgb[0])/C + 2;
	}else{
		Hp = (rgb[0] - rgb[1])/C + 4;
	}
	H = 60.0f*Hp;
	L = (M + m)/2.0f;	
	//V = M;
	//HSL
	if(C==0.0){
		S = 0.0;
	}else if(L <= 0.5f){
		S = C/(2.0f*L);
	}else{
		S = C/(2.0f-2.0f*L);
	}
	hsl[0] = H/360.0f;
	hsl[1] = S;
	hsl[2] = L;
}
/* 	hlPrintColor(...) 		*/
void hlPrintColor(const hlColor *color){
	int max = hlCSGetChan(hlColorGetCS(color));
	int i = 0;
	printf("<hlColor> \n");
	if(color){
		hlPrintCS(hlColorGetCS(color));
		printf("   ");
		while(i < max){
			printf("%f, ",hlColorGetChan(color,i));
			i++;
		}
		printf("</hlColor>\n");
		return;
	}
	else{
		printf("   NULL \n </hlColor>\n");
		return;
	}
}

