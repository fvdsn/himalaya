#include "hlTile.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

long int random(void);	/*TODO wtf do we need this ? */
extern int num_tile;

hlTile *hlNewTile(hlCS cs){
	hlTile *tile;
	tile = (hlTile*)malloc(hlTileSize(cs));
	assert(tile);
	num_tile++;
	return tile;
}
void hlFreeTile(hlTile *tile){
	//fprintf(stdout,"hlFreeTile(%p)\n",(void*)tile);
	num_tile--;
	free(tile);
}
int hlTileSize(hlCS cs){
	int size = 0;
	switch(hlCSGetBpc(cs)){ 
		case HL_8B:
			switch (hlCSGetChan(cs)){
				case 1: {size=HL_TILEWIDTH_8B1;break;}
				case 2: {size=HL_TILEWIDTH_8B2;break;}
				case 3: {size=HL_TILEWIDTH_8B3;break;}
				case 4: {size=HL_TILEWIDTH_8B4;break;}
				case 5: {size=HL_TILEWIDTH_8B5;break;}
			}
		break;
		case HL_32B:
			switch (hlCSGetChan(cs)){
				case 1: {size=HL_TILEWIDTH_32B1;break;}
				case 2: {size=HL_TILEWIDTH_32B2;break;}
				case 3: {size=HL_TILEWIDTH_32B3;break;}
				case 4: {size=HL_TILEWIDTH_32B4;break;}
				case 5: {size=HL_TILEWIDTH_32B5;break;}
			}	
		break;
		
	}
	if(!size){
		hlPrintCS(cs);
	}
	assert(size);
	return size;
}
void hlTileZeroes(hlTile *tile, hlCS cs){
	memset(tile,0,hlTileSize(cs));
}
void hlTileRandom(hlTile *tile, hlCS cs){
	uint8_t* d8 	= HL_DATA_8B(tile);
	int i = hlTileSize(cs);
	while(i--){
		d8[i] = (uint8_t)(random()%255);
	}
}
void hlTileFill(hlTile *tile, hlColor *col){
	uint8_t* d8 	= HL_DATA_8B(tile);
	const int bpp  = hlCSGetBpp(hlColorGetCS(col));
	int i = HL_TILE_PIXCOUNT;
	while(i--){
		memcpy(d8+HL_INDEX(i,bpp),col->color,bpp);
	}
}
void	hlTileColorPick(hlTile *tile, hlCS cs,int x, int y, hlColor *color){
	uint8_t* d8  = HL_DATA_8B(tile);
	int chan = hlCSGetChan(cs);
	int bpp  = hlCSGetBpp(cs);
	int c = chan;
	color->cs = cs;
	if(x < 0 || x >= HL_TILEWIDTH){
		fprintf(stderr,"ERROR, colorpicking x outside tile bounds :%d\n",x);
		return;
	}
	if(y < 0 || y >= HL_TILEWIDTH){
		fprintf(stderr,"ERROR, colorpicking y outside tile bounds :%d\n",y);
		return;
	}
	while(c--){
		int clr = d8[(y*HL_TILEWIDTH+x)*bpp + c];
		printf("%d,",clr);
		hlColorSetChan(color,c,clr/255.0);
	}
	printf("\n");
}
void hlTileMult(hlTile *tile, hlColor *col){
	uint8_t* d8 	= HL_DATA_8B(tile);
	const int bpp = hlCSGetBpp(hlColorGetCS(col));
	const uint8_t alpha = col->color[bpp-1];
	int i = HL_TILE_PIXCOUNT;
	while(i--){//TODO MAKE IT WORK FOR 32BITS
		int c = bpp-1;
		while(c--){
			d8[i*bpp+c] = (d8[i*bpp+c]*col->color[c]*alpha)/(255*255);
		}
	}
}
void hlTileCopy(hlTile *dst, const hlTile *src, hlCS cs){
	memcpy(dst,src,hlTileSize(cs));
}
void	hlTileInterp(hlTile *dst, hlCS cs,const hlTile *tl, const hlTile *tr, 
			const hlTile *br, const hlTile *bl){
	uint8_t*dst8		= HL_DATA_8B(dst);
	const uint8_t*tl8	= HL_DATA_8B(tl);
	const uint8_t*tr8	= HL_DATA_8B(tr);
	const uint8_t*br8	= HL_DATA_8B(br);
	const uint8_t*bl8	= HL_DATA_8B(bl);
	int chan = hlCSGetChan(cs);
#define H  HL_TILEWIDTH/2
	int x = H;
	while(x--){
		int y = H;
		while(y--){
			int c = chan;
			while(c--){
				dst8[HL_XYC(x,y,c)] = 	(tl8[HL_XYC(x,y,c)] + tl8[HL_XYC(x+1,y,c)]
							+ tl8[HL_XYC(x+1,y+1,c)] + tl8[HL_XYC(x,y+1,c)])/4;
				dst8[HL_XYC(x+H,y,c)] =	(tr8[HL_XYC(x,y,c)] + tr8[HL_XYC(x+1,y,c)]
							+ tr8[HL_XYC(x+1,y+1,c)] + tr8[HL_XYC(x,y+1,c)])/4;
				dst8[HL_XYC(x+H,y+H,c)] =	(br8[HL_XYC(x,y,c)] + br8[HL_XYC(x+1,y,c)]
							+ br8[HL_XYC(x+1,y+1,c)] + br8[HL_XYC(x,y+1,c)])/4;
				dst8[HL_XYC(x,y+H,c)] =	(bl8[HL_XYC(x,y,c)] + bl8[HL_XYC(x+1,y,c)]
							+ bl8[HL_XYC(x+1,y+1,c)] + bl8[HL_XYC(x,y+1,c)])/4;
			}
		}
	}			
}
float hlTileGetXYC( 	hlTile *tile,
			hlCS cs,
			unsigned int x,
			unsigned int y,
			unsigned int chan){
	if(HL_8B == hlCSGetBpc(cs))
		return (float)(HL_DATA_8B(tile)[HL_XYC(x,y,chan)]) / 255.0;
	else
		return HL_DATA_32B(tile)[HL_XYC(x,y,chan)];
}
hlTile *hlTileDup(const hlTile *src, hlCS cs){
	hlTile *tile = hlNewTile(cs);
	memcpy(tile,src,hlTileSize(cs));
	return tile;
}
static char hl_float_to_ascii(float intensity){
	char table[8] = {'.',
			'^',
			':',
			'*',
			'x',
			'%',
			'@',
			'#'};
	intensity = intensity > 1.0 ? 1.0 : intensity ;
	intensity = intensity < 0.0 ? 0.0 : intensity ;
	return table[(int)(intensity*7)];
}
void hlPrintTile(hlTile *tile, hlCS cs){
	int i = 0;
	int j = 0;
	int c = 0;
	float acc = 0.0;
	printf("<hlTile>");
	do{
		j = 0;
		do{
			acc = 0.0;
			c = 0;
			do{
				acc += hlTileGetXYC(tile,cs,j,i,c);
			}while(++c < hlCSGetChan(cs));
			printf("%c",hl_float_to_ascii(acc/hlCSGetChan(cs)));
			/*printf("%d",(int)acc);*/
		}while(++j < HL_TILEWIDTH);
		printf("\n");
	}while(++i < HL_TILEWIDTH);
	printf("</hlTile>\n");
	return;
}
void hlPrintTileMem(hlTile *tile, hlCS cs){
	uint8_t *data = HL_DATA_8B(tile);
	const int bpp   = hlCSGetBpp(cs);
	const int chan 	= hlCSGetChan(cs);
	int i = 0;
	int x = 0;
	int y = 0;
	int c = 0;
	int p = 0;
	printf("<hlTile>\n");
	printf("Tile Size in bytes : %d\n",hlTileSize(cs));
	printf("Byte per Channel   : %d\n",bpp /chan );
	printf("Byte per Pixel     : %d\n",bpp);
	printf("Channel Count      : %d\n",chan);
	do{
		y = 0;
		do{
			c = 0;
			do {
				p = 0;
				do {
					 printf("%d.",data[i]);
					 i++;
				}while(++p < (bpp/chan));
				printf(" ");
			}while(++c < chan);
		printf("|");
		}while(++y < HL_TILEWIDTH);
	printf("\n");
	printf("</hlTile>\n");
	}while(++x < HL_TILEWIDTH);
}	

	
