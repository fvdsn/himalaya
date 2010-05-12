#ifndef __HL_REGION_H__
#define __HL_REGION_H__

typedef struct hl_region{
	int px; /*index of the top left pixel in region*/
	int py;
	int sx; /* width/ height of region */
	int sy;
	int tx; /*index of the bottom right tile in region */
	int ty;
	int ptx;/*index of the top left tile in region */ 
	int pty;
	int z; /*zoom level of region : 0 = orignal, 1 =
			2x zoom, 2 = 4x zoom, etc.*/
}hlRegion;

hlRegion hlNewRegion(	int px,
			int py,
			int sx,
			int sy,
			int z	);
void hlPrintRegion(hlRegion r);
int  hlTileCoordFromCoord(int x);

#endif
