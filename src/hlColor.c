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
		case HL_8B : {
			col->color[channel] = (uint8_t)(value*255.0);
			break;
		}
		case HL_32B : {
			col32f[channel] = value;
			break;
		}
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

