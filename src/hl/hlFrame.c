#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hlFrame.h"

extern int num_frame;
extern int num_frame_node;



/*	hlNewFrame(...)		*/
static unsigned int hl_depth_from_size(int ptx, int pty, int tx, int ty){
	unsigned int depth = 0;
	tx = tx - ptx;	/*how many tiles in a line minus one*/
	ty = ty - pty;
	if (ty > tx){ tx = ty;}
	assert(tx > 0 );
	while(tx){
		tx = tx/2;
		depth++;
	}
	return depth;
}
static int hl_zoom_size(int size, unsigned int z){
	while(z--){
		size /=2;
	}
	return size;
}
static hlNode* hl_new_node(uint32_t x, uint32_t y){
	hlNode* n = (hlNode*)malloc(sizeof(hlNode));
	memset(n,0,sizeof(hlNode));
	n->x = x;
	n->y = y;
	num_frame_node++;
	return n;
}
hlFrame* hlNewFrame(hlColor bgcol, int sx, int sy){
	hlFrame *f = (hlFrame*)malloc(sizeof(hlFrame));
	f->region = hlNewRegion(0,0,sx,sy,0);
	f->tlroot = hl_new_node(0,0);
	f->trroot = hl_new_node(0,0);
	f->brroot = hl_new_node(0,0);
	f->blroot = hl_new_node(0,0);
	f->depth = hl_depth_from_size(	f->region.ptx,
					f->region.pty,
					f->region.tx,
					f->region.ty	); 
	hlColorCopy(&f->color,&bgcol);
	f->cs = hlColorGetCS(&bgcol);
	f->bg = hlNewTile(hlColorGetCS(&bgcol));
	hlTileFill(f->bg,&bgcol);
	num_frame++;
	return f;
}

/* 	hlFreeFrame(...) 	*/
static void hl_free_node_tree(hlNode *n){
	if(n){
		hl_free_node_tree(n->tl);
		hl_free_node_tree(n->tr);
		hl_free_node_tree(n->br);
		hl_free_node_tree(n->bl);
		if(n->tile){
			hlFreeTile(n->tile);
		}
		num_frame_node--;
		free(n);
	}
}
void hlFramePurge(hlFrame *f){
	hl_free_node_tree(f->tlroot);
	hl_free_node_tree(f->trroot);
	hl_free_node_tree(f->brroot);
	hl_free_node_tree(f->blroot);
	f->tlroot = hl_new_node(0,0);
	f->trroot = hl_new_node(0,0);
	f->brroot = hl_new_node(0,0);
	f->blroot = hl_new_node(0,0);
}
void hlFreeFrame(hlFrame *f){
	if(f){
		hl_free_node_tree(f->tlroot);
		hl_free_node_tree(f->trroot);
		hl_free_node_tree(f->brroot);
		hl_free_node_tree(f->blroot);
		hlFreeTile(f->bg);
		num_frame--;
		free(f);
	}
}

/* 	hlFrameSizeXY(..) / hlFrameTileXY(..) / etc ... */
/* frame have infinite size, this provides a quick way to get interesting
 * data from the frames render region */
unsigned int hlFrameSizeX(hlFrame *f, unsigned int z){
	return hl_zoom_size(f->region.sx,z);
}
unsigned int hlFrameSizeY(hlFrame *f, unsigned int z){
	return hl_zoom_size(f->region.sy,z);
}
unsigned int hlFrameTileX(hlFrame *f, unsigned int z){
	hlRegion r = hlNewRegion(0,0,hlFrameSizeX(f,z),hlFrameSizeY(f,z),z);
	return r.tx;
}
unsigned int hlFrameTileY(hlFrame *f, unsigned int z){
	hlRegion r = hlNewRegion(0,0,hlFrameSizeX(f,z),hlFrameSizeY(f,z),z);
	return r.ty;
}
unsigned int hlFrameDepth(hlFrame *f){
	return f->depth;
}
hlCS hlFrameCS(hlFrame *f){
	return f->cs;
}
hlColor hlFrameColor(hlFrame *f){
	return f->color;
}

/* 	hlFrameTileGet/Read/Copy(...) 	*/
static int depth_from_tile(int x, int y, int z){
	/* given a x,y,z tile coordinates, returns the minimum quadtree
	 * depth so that it can store the tile */
	int tmp = z;
	int depth = 0;
	if(x < 0){x = -x-1;}
	if(y < 0){y = -y-1;}
	if(y > x){x = y;}
	while(tmp--){
		x = x * 2 + 1;
	}/*now we have the x = the greatest coordinate of a tile at z == 0*/
	while(x){
		x = x/2;
		depth++;
	}
	return depth;
}
static hlNode *hl_node_add_level(hlNode*n,int i){
	/*given the root node of a tree, adds i level on top of
	 * that root node, and returns the new root*/
	hlNode*new;
	if (i==0){
		return n;
	}else{
		new = hl_new_node(0,0);
		new->tl = hl_node_add_level(n,i-1);
		return new;
	}
}		
static void hl_frame_grow(hlFrame *f, int x, int y, int z){
	/*grows the frame depth so that it can store a tile
	 * at given coordinates 
	 * TODO : don't grow the octree if it's empty */
	unsigned int newdepth = depth_from_tile(x,y,z);
	if(newdepth <= f->depth){
		return;
	}else{
		f->depth = newdepth;
		f->tlroot = hl_node_add_level(f->tlroot, newdepth - f->depth);
		f->trroot = hl_node_add_level(f->trroot, newdepth - f->depth);
		f->brroot = hl_node_add_level(f->brroot, newdepth - f->depth);
		f->blroot = hl_node_add_level(f->blroot, newdepth - f->depth);
	}
}
static hlNode* hl_tile_match(hlNode* n, unsigned int x, unsigned int y){
	/* returns the child node of n matching the coordinates x,y. returns null if
	 * nothing matches
	 */
	if (n->tl && n->tl->x == x && n->tl->y == y)
		return n->tl;
	else if (n->tr && n->tr->x == x && n->tr->y == y)
		return n->tr;
	else if (n->br && n->br->x == x && n->br->y == y)
		return n->br;
	else if (n->bl && n->bl->x == x && n->bl->y == y)
		return n->bl;
	else
		return NULL;
}
static int hl_tile_path(unsigned int *pathx, unsigned int *pathy, int tx, int ty, int z, int depth){
	int i = depth - z;
	/*in the quadtree, coordinates are always positive, negatives
	 * coordinates are simulated with a different quadtree for each quadrant
	 * the coordinates for the path reflect the inner representation of
	 * each octree. If you find a cleaner solution, I'm all for it
	 * it returns 1 if the tile could'nt possibly be in the quadtree*/
	if(tx < 0){ tx = -tx-1;}; 
	if(ty < 0){ ty = -ty-1;};
	if(z > depth || z >= HL_MAX_TILE_LEVEL){ i = 0;}
	if(i <= 0){
		if(tx || ty){
			return 1;
		}
		return 0;
	}
	do{
		pathx[i] = tx;
		pathy[i] = ty;
		tx = tx/2;
		ty = ty/2;
	}while(i--);
	if(pathx[0] || pathy[0]){
		return 1;
	}else{
		return 0;
	}
}
hlTile* hlFrameTileGet(hlFrame *f, int x, int y,unsigned int z){
	hlNode *n;
	unsigned int pathx[HL_MAX_TILE_LEVEL];
	unsigned int pathy[HL_MAX_TILE_LEVEL];
	int level = f->depth-z;
	int i = level;
	int j = 0;
	if(hl_tile_path(pathx,pathy,x,y,z,f->depth)){
		return NULL;
	}else{
		/*there is a quadtree for each quadrant*/
		if( x >= 0){	if(y>=0){n = f->brroot;}	
				else{	 n = f->trroot;}
		}else{		if(y>=0){n = f->blroot;}
				else{	 n = f->tlroot;}
		}
		if(i<=0){/*return root for z too large*/
			return n->tile;
		}
		while(i--){/*tile is in a child node*/
			j++;
			n = hl_tile_match(n,pathx[j],pathy[j]);
			if(n == NULL)
				return NULL;
		}
	}
	return n->tile;	/*TODO is the depth awkay ?*/
}
hlTile* hlFrameTileRead(hlFrame* f, int x, int y, unsigned int z){
	hlTile* t = hlFrameTileGet(f,x,y,z);
	if(t)
		return t;
	else
		return f->bg;
}
hlTile* hlFrameTileCopy(hlFrame* f, int x, int y, unsigned int z){
	return hlTileDup(hlFrameTileRead(f,x,y,z),hlFrameCS(f));
}	

/* 	hlFrameTileSet(...) 	*/
static hlNode* hl_tile_match_set(hlNode* n,uint32_t u,uint32_t v,uint32_t x,uint32_t y ){
	/* returns the child node of n matching the coordinates x,y.
	 * if the node doesn't exists, it is created.
	 * u,v are the coordinates of the node n
	 *
	 * I coded this in a misterious mood, it looks
	 * like it does three time as much as needed, but it should
	 * work fine :p
	 */
	uint32_t pu = u;
	uint32_t pv = v;
	assert(n->x == pu && n->y == pv);
	u = u*2 +1 ;
	v = v*2 +1 ;
	if( x < u && y < v ){
		if (n->tl && n->tl->x == x && n->tl->y == y){
			return n->tl;
		}else{
			assert(!n->tl);
			return n->tl = hl_new_node(x,y);
		}
	}
	else if( x == u && y < v ){
		if (n->tr && n->tr->x == x && n->tr->y == y){
			return n->tr;
		}else{
			assert(!n->tr);
			return n->tr = hl_new_node(x,y);
		}
	}
	else if( x == u && y == v ){
		if (n->br && n->br->x == x && n->br->y == y){
			return n->br;
		}else{
			assert(!n->br);
			return n->br = hl_new_node(x,y);
		}
	}
	else if( x < u && y == v ){
		if (n->bl && n->bl->x == x && n->bl->y == y){
			return n->bl;
		}else{
			assert(!n->bl);
			return n->bl = hl_new_node(x,y);
		}
	}else{ 
		printf("pu:%d pv:%d | u:%d v:%d | x:%d y:%d\n",pu,pv,u,v,x,y);
		assert(0 && "big screwup");
		return NULL;
	}
}
static hlNode *hl_frame_get_root(hlFrame *f, int x, int y){
	if( x >= 0){
		if(y>=0){return f->brroot;}	
		else{	 return f->trroot;}
	}else{
		if(y>=0){return f->blroot;}
		else{	 return f->tlroot;}
	}
}
void hlFrameTileSet(hlFrame *f, hlTile * tile,int x, int y, unsigned int z){
	hlNode *n = hl_frame_get_root(f,x,y);
	uint32_t pathx[HL_MAX_TILE_LEVEL];
	uint32_t pathy[HL_MAX_TILE_LEVEL];
	uint32_t level = f->depth - z;
	uint32_t i = level;
	uint32_t j = 0;
	if(hl_tile_path(pathx,pathy,x,y,z,f->depth)){
		hl_frame_grow(f,x,y,z);		return;
	}
	while(i--){
		j++;
		n = hl_tile_match_set(n,pathx[j-1],pathy[j-1],
					pathx[j],pathy[j]);
	}
	if (n->tile)
		hlFreeTile(n->tile);
	n->tile = tile;
	return;
}

/* 	hlFrameTileRemove(...) 	*/
static void hl_remove_child_node(hlNode *node, hlNode *parent){
	/* removes the link between node and parent, and
	 * frees the node */
	if(parent->tl == node){
		parent->tl = NULL;
		free(node);
	}else if(parent->tr == node){
		parent->tr = NULL;
		free(node);
	}else if(parent->br == node){
		parent->br = NULL;
		free(node);
	}else if(parent->bl == node){
		parent->bl = NULL;
		free(node);
	}else{
		assert(0 && "frame insanity");
	}
}
hlTile * hlFrameTileRemove(hlFrame *f, int x, int y, unsigned int z){
	hlNode *n = hl_frame_get_root(f,x,y);
	hlTile * t = NULL;
	unsigned int pathx[HL_MAX_TILE_LEVEL];
	unsigned int pathy[HL_MAX_TILE_LEVEL];
	hlNode * pathn[HL_MAX_TILE_LEVEL];	/*list of node from root to removed node */
	int level = f->depth - z;
	int i = level;
	int j = 0;
	/* creates a theorical quadtree path to the tile */
	if(hl_tile_path(pathx,pathy,x,y,z,f->depth)){
		return NULL;
	}
	/*create the list of nodes from root to tile */
	pathn[0] = n;
	while(i--){
		j++;
		n = hl_tile_match(n,pathx[j],pathy[j]);
		pathn[j] = n;
		if(!n){	
			/* target node not in quadtree */
			return NULL;
		}
		assert(n);
	}
	assert(n->tile);
	t = n->tile;
	n->tile = NULL;
	/* if the node where we removed the tile doesn't have any more tiles,
	 * we free it. And that all the way to root */
	do{
		if(	!pathn[j]->tl && !pathn[j]->tr 
			&& !pathn[j]->br && !pathn[j]->bl
			&& !pathn[j]->tile	)
			
			hl_remove_child_node(pathn[j],pathn[j-1]);
		else
			break;
	}while(j-- > 1);
	return t;
}
void hlFrameTileFree(hlFrame *f, int x, int y, unsigned int z){
	hlTile *t = hlFrameTileRemove(f,x,y,z);
	if (t){
		hlFreeTile(t);
	}
}
/*static void hl_print_path( uint32_t* pathx, uint32_t* pathy, int n){
	int i = 0;
	while(i < n+1){
		printf("%d,%d\n",pathx[i], pathy[i]);
		i++;
	}
}*/

/* 	hlRawFromFrame(...) 	*/
/*
static hlTile *hl_black_rgba_tile(){
	hlCS cs = hlNewCS(HL_8B,HL_RGB);
	hlColor c = hlNewColor(cs,0,0,0,0,255);
	hlTile *t = hlNewTile(cs);
	hlTileFill(t,&c);
	return t;
}
static hlTile *hl_white_rgba_tile(){
	hlCS cs = hlNewCS(HL_8B,HL_RGB);
	hlColor c = hlNewColor(cs,255,255,255,0,255);
	hlTile *t = hlNewTile(cs);
	hlTileFill(t,&c);
	return t;
}*/
static void hlTileToRaw(hlTile *t, hlRaw *raw, int px, int py){
	const int sx = hlRawSizeX(raw);
	const int sy = hlRawSizeY(raw);
	const int bpp = hlCSGetBpp(hlRawCS(raw));
	uint8_t * tiledata = HL_DATA_8B(t);
	int ptx = 0;	/* top left point on tile */
	int pty = 0;
	int prx = px;	/* top left point on raw */
	int pry = py;
	int psx = HL_TILEWIDTH;	/* size of copy region */
	int psy = HL_TILEWIDTH;
	int y;
	if(px <= -HL_TILEWIDTH || py <= -HL_TILEWIDTH || px >= sx || py >=sy ||sx <= 0 || sy <=0){
		printf("rejected px,py:%d,%d\n",px,py);
		return;
	}
	if(px < 0){ 
		ptx = -px;
		psx += px;
		prx = 0;
	}
	if(py < 0){ 
		pty = -py;
		psy += py;
		pry = 0;
	}
	if(px + HL_TILEWIDTH > sx){
		psx -= (px + HL_TILEWIDTH - sx);
	}
	if(py + HL_TILEWIDTH > sy){
		psy -= (py + HL_TILEWIDTH - sy);
	}
	y = psy;
	if(psy <= 0 ){
		printf("rejected\n");
	}
	while(y-- > 0){
		memcpy(	raw->data +(prx+sx*(pry+y))*bpp,
			tiledata + (ptx+HL_TILEWIDTH*(pty+y))*bpp,
			psx*bpp);
	}
}

void hlRegionToRaw(hlFrame *f,hlRaw *raw,int px, int py,unsigned int z){
	hlRegion r = hlNewRegion(px,py,hlRawSizeX(raw),hlRawSizeY(raw),z);
	int x,y;	/*tile index*/
	int rx,ry;	/*top left pixel in raw where current tile will be copied */
	x = r.tx;	
	while(x--){
		y = r.ty;
		while(y--){
			rx = (r.ptx+x)*HL_TILEWIDTH -px;
			ry = (r.pty+y)*HL_TILEWIDTH -py;
			hlTileToRaw(	hlFrameTileRead(f,r.ptx+x,r.pty+y, r.z ),
					raw,
					rx,
					ry);
		}
	}
}
/*
static void hlRegionToRaw2(hlFrame *f,hlRaw *raw,int px, int py,unsigned int z){
	hlRegion r = hlNewRegion(px,py,hlRawSizeX(raw),hlRawSizeY(raw),z);
	const int bpp = hlCSGetBpp(hlFrameCS(f));
	uint8_t * tiledata;
	int x; /rx*tile indexx/
	int y;
	int rx; /top left pixel in raw where current tile will be copied. >=0 /
	int ry;
	int trx;/top left pixel in tile from where it will be copied. in [0 HL_TILEWIDTH -1]/
	int try;
	int mx; /width of the line/collums in tile to be copied. in [1 HL_TILEWIDTH]/
	int my;
	int i;
	int a,b,c;
	c = hlRawSizeX(raw)*hlRawSizeY(raw)*bpp;
	x = r.tx;
	/hlRawFill(raw,&bgc)/;
	hlPrintRegion(r);
	printf("%p\n",f->bg);
	while(x--){
		y = r.ty;
		while(y--){
			rx = (r.ptx+x)*HL_TILEWIDTH - r.px;
			ry = (r.pty+y)*HL_TILEWIDTH - r.py;
			if(rx < 0) {rx = 0;}
			if(ry < 0) {ry = 0;}
			if(!x){ trx = r.px - (r.ptx * HL_TILEWIDTH );}
			else {  trx = 0;}
			if(!y){ try = r.py - (r.pty * HL_TILEWIDTH );}
			else {  try = 0;}
			mx = HL_TILEWIDTH - trx;
			my = HL_TILEWIDTH - try;
			if (x == r.tx - 1){
				mx -= (r.ptx+r.tx)*HL_TILEWIDTH - (r.px+r.sx);
			}
			if( y == r.ty - 1){
				my -= (r.pty+r.ty)*HL_TILEWIDTH - (r.py+r.sy);
			}
			i = my;
			tiledata = HL_DATA_8B(hlFrameTileRead(  f,
								r.ptx+x,
								r.pty+y,
								r.z ));
			if(i > HL_TILEWIDTH){
				printf("i too big ...\n");
				i = HL_TILEWIDTH;
			}
			while(i-- > 0){
				if(!tiledata){
					printf("no tiledata\n");
				}
				a = ((ry+i)*r.sx + rx)*bpp;
				b = ((try+i)*HL_TILEWIDTH + trx)*bpp;
				if(a <0 || a > c ) {
					printf("ry: %d \ti:%d \tr.sx:%d \trx:%d \ta:%d\n",ry,i,r.sx,rx,a);
				}
				if(b < 0 || b > HL_TILE_PIXCOUNT*bpp) {
					printf("ry: %d \ti:%d \tr.sx:%d \trx:%d \tb:%d\n",ry,i,r.sx,rx,b);
				}

				//printf("%d,%d,%d\n", ((try+i)*HL_TILEWIDTH +trx)*bpp,mx*bpp);
				memcpy(raw->data + ((ry+i)*r.sx + rx)*bpp,
					tiledata + ((try+i)*HL_TILEWIDTH + trx)*bpp,
					mx*bpp);
			}
		}
	}
}
*/
hlRaw *hlRawFromRegion(hlFrame *f, hlRegion r){
	hlRaw *raw = hlNewRaw(hlFrameCS(f),r.sx,r.sy);
	hlRegionToRaw(f,raw,r.px,r.py,r.z);
	return raw;
}
hlRaw * hlRawFromFrame(hlFrame *f,unsigned int z){
	hlRegion r = hlNewRegion(0,0,hlFrameSizeX(f,z),hlFrameSizeY(f,z),z);
	return hlRawFromRegion(f,r);
}
	
/* 	hlFrameFromRaw(...) 	*/
static hlTile *hlTileFromRaw(hlRaw *r,hlColor *bg,int tx, int ty){
	hlTile* t = hlNewTile(r->cs);
	const int bpp = hlCSGetBpp(r->cs);
	const int px  = tx * HL_TILEWIDTH;
	const int py  = ty * HL_TILEWIDTH;
	const int Y   = py + HL_TILEWIDTH > r->sy ?
				r->sy - py : HL_TILEWIDTH;
	const int X   = px + HL_TILEWIDTH > r->sx ?
				(r->sx - px) * bpp :
				HL_TILEWIDTH * bpp;
	int i = Y;
	if(X!=HL_TILEWIDTH*bpp || Y!= HL_TILEWIDTH*bpp){
		hlTileFill(t,bg);
	}
	while (i--){
		memcpy(	(char*)(HL_DATA_8B(t)) + HL_TILEWIDTH * i *bpp,
			(char*)(r->data) + ((py+i)*r->sx + px) * bpp, X);
	}
	return t;
}
hlFrame *hlFrameFromRaw(hlRaw *r){
	hlColor c  = hlNewColor(r->cs,0,0,0,0,0);
	hlFrame *f = hlNewFrame(c,r->sx,r->sy);
	const uint32_t tx = hlFrameTileX(f,0);
	const uint32_t ty = hlFrameTileY(f,0);
	uint32_t x = tx;
	uint32_t y = ty;
	while(y--){
		x = tx;
		while(x--){
			hlFrameTileSet(f,hlTileFromRaw(r,&c,x,y),x,y,0);
		}
	}
	return f;
}

/* 	hlFrameMakeMipmap(...) 	*/
static void hl_tile_cpy_pixel(hlCS cs, hlTile* dst,uint32_t dx, uint32_t dy, const hlTile *src, uint32_t sx, uint32_t sy){
	const uint32_t bpp = hlCSGetBpp(cs);
	memcpy(	(char*)(HL_DATA_8B(dst)) + ((dy * HL_TILEWIDTH) + dx)*bpp,
		(char*)(HL_DATA_8B(src)) + ((sy * HL_TILEWIDTH) + sx)*bpp,
		bpp);
}
static void hl_tile_mipmap(hlCS cs, hlTile* dst, const hlTile *src, int top, int left){
	const uint32_t px = left ? 0 : HL_TILEWIDTH / 2 ;
	const uint32_t py = top  ? 0 : HL_TILEWIDTH / 2 ;
	uint32_t x = HL_TILEWIDTH / 2;
	uint32_t y = HL_TILEWIDTH / 2;
	while(x--){
		y = HL_TILEWIDTH / 2;
		while (y--){
			hl_tile_cpy_pixel(cs,	dst, x + px, y + py, 
						src, x * 2 , y * 2  );
		}
	}
}
static void hl_node_source_mipmap(hlNode* n,hlCS cs){
	if(!n->tile){
		n->tile = hlNewTile(cs);

		if(n->tl){
			if(!n->tl->tile)
				hl_node_source_mipmap(n->tl,cs);
			hl_tile_mipmap(cs,n->tile,n->tl->tile,1,1);
		}
		if(n->tr){
			if(!n->tr->tile)
				hl_node_source_mipmap(n->tr,cs);
			hl_tile_mipmap(cs,n->tile,n->tr->tile,1,0);
		}
		if(n->br){
			if(!n->br->tile)
				hl_node_source_mipmap(n->br,cs);
			hl_tile_mipmap(cs,n->tile,n->br->tile,0,0);
		}
		if(n->bl){
			if(!n->bl->tile)
				hl_node_source_mipmap(n->bl,cs);
			hl_tile_mipmap(cs,n->tile,n->bl->tile,0,1);
		}
	}
}
void 	hlFrameMipMap(hlFrame *f){
	hl_node_source_mipmap(f->tlroot,hlFrameCS(f));
	hl_node_source_mipmap(f->trroot,hlFrameCS(f));
	hl_node_source_mipmap(f->brroot,hlFrameCS(f));
	hl_node_source_mipmap(f->blroot,hlFrameCS(f));
}
void hlPrintFrame(hlFrame *f){
	printf("<frame>\n");
	printf("cs:\n");
	hlPrintCS(f->cs);
	printf("color:\n");
	hlPrintColor(&(f->color));
	printf("region:\n");
	hlPrintRegion(f->region);
	printf("depth:%d\n",f->depth);
	printf("</frame>\n");
	return;

}
/*int main(int argc, char** argv){
	hlCS cs = hlNewCS(HL_8B,HL_RGB);
	hlRaw *in  = hlRawFromPng("blending.png");
	hlRegion r = hlNewRegion(-16,-16,47,47,0);
	hlRaw *out = NULL;
	hlFrame *f = hlFrameFromRaw(in);
	hlPrintFrame(f);
	hlPrintRegion(r);
	hlFrameMipMap(f);
	out = hlRawFromFrame(f,0);
	hlRawToPng(out,"f-out.png");
	return 1;
}*/
