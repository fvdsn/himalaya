#include <stdlib.h>
#include <string.h>
#include "uiHl.h"

uiEntity *uiHlNew(const char *name, hlImg *img, hlState s,hlRaw *raw){
	uiEntity *h = uiEntityNew(name,UI_ENT_HL);
	uiHlData *hd = (uiHlData*)malloc(sizeof(uiHlData));
	memset((char*)hd,0,sizeof(uiHlData));
	h->data = hd;
	hd->img = img;
	hd->state = s;
	hd->raw = raw;
	hd->zoom = 1.0f;
	return h;
}
void	uiHlSetImg(uiEntity *hl, hlImg *img){
	/*TODO assert correct entity type */
	((uiHlData*)hl->data)->img = img;
}
void	uiHlSetState(uiEntity *hl, hlState s){
	/*TODO assert correct entity type */
	((uiHlData*)hl->data)->state = s;
}

	
