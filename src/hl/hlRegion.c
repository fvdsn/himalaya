#include <stdio.h>
#include "hlRegion.h"
#include "hlTile.h"

/* returns the tile where the coordinate sx is included */
int hl_txy_from_sxy(int sx){
	if(sx >= 0)
		return sx/HL_TILEWIDTH;
	else
		return (sx+1)/HL_TILEWIDTH - 1;
}
hlRegion hlNewRegion(int px, int py, int sx, int sy, unsigned int z){
	hlRegion r;
	if (sx <= 0){sx = 1;}
	if (sy <= 0){sy = 1;}
	r.z = z;
	r.px = px;
	r.py = py;
	r.sx = sx;
	r.sy = sy;
	r.ptx = hl_txy_from_sxy(px);
	r.pty = hl_txy_from_sxy(py);
	r.tx  = hl_txy_from_sxy(px + sx - 1) - r.ptx + 1;
	r.ty  = hl_txy_from_sxy(py + sy - 1) - r.pty + 1;
	return r;
}
void hlPrintRegion(hlRegion r){
	printf("<region>\n");
	printf("px,py:(%d.%d)\n",r.px,r.py);
	printf("sx,sy:(%d.%d)\n",r.sx,r.sy);
	printf("tx,ty:(%d.%d)\n",r.tx,r.ty);
	printf("ptx,pty:(%d.%d)\n",r.ptx,r.pty);
	printf("z:%d\n",r.z);
	printf("</region>\n");
}
/*
int main(int argc, char**argv){
	hlRegion r1 = hlNewRegion(0,0,64,32,0);
	hlRegion r2 = hlNewRegion(0,0,65,31,0);
	hlRegion r3 = hlNewRegion(0,0,80,20,0);
	hlRegion r4 = hlNewRegion(32,64,50,50,0);
	hlRegion r5 = hlNewRegion(-32,-65,50,50,0);
	hlPrintRegion(r1);
	hlPrintRegion(r2);
	hlPrintRegion(r3);
	hlPrintRegion(r4);
	hlPrintRegion(r5);
	return 0;
}*/
