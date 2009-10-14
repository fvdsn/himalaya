#include "hlTile.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

long int random();

hlTile *hlNewTile(hlCS cs){
	hlTile *tile;
	tile = (hlTile*)malloc(hlTileSize(cs));
	assert(tile);
	return tile;
}
void hlFreeTile(hlTile *tile){
	free(tile);
}
int hlTileSize(hlCS cs){
	int size = 0;
	switch(hlCSGetBpc(cs)){ 
		case HL_8B:{
			switch (hlCSGetChan(cs)){
				case 1: {size=HL_TILEWIDTH_8B1;break;}
				case 2: {size=HL_TILEWIDTH_8B2;break;}
				case 3: {size=HL_TILEWIDTH_8B3;break;}
				case 4: {size=HL_TILEWIDTH_8B4;break;}
				case 5: {size=HL_TILEWIDTH_8B5;break;}
			}
		}
		case HL_32B:{
			switch (hlCSGetChan(cs)){
				case 1: {size=HL_TILEWIDTH_32B1;break;}
				case 2: {size=HL_TILEWIDTH_32B2;break;}
				case 3: {size=HL_TILEWIDTH_32B3;break;}
				case 4: {size=HL_TILEWIDTH_32B4;break;}
				case 5: {size=HL_TILEWIDTH_32B5;break;}
			}	
		}
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
	float* d32 	= HL_DATA_32B(tile);
	const int chan = hlCSGetChan(cs);
	int c = chan;
	int i = HL_TILE_PIXCOUNT*chan;
	while(i--){
		c = chan;
		while(c--){
			if(HL_8B == hlCSGetChan(cs))
				d8[i] = (uint8_t)(random()%255);
			else
				d32[i] = (float)(random()%100000)/100000.0;
		}
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
void hlTileCopy(hlTile *dst, const hlTile *src, hlCS cs){
	memcpy(dst,src,hlTileSize(cs));
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
	