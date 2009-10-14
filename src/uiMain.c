#include <stdlib.h>
#include <stdio.h>
#include "gui/uiGui.h"
static uiEntity *uiSideBar(void){
	uiEntity *te;
	uiEntity *t;
	te = uiTabEnvNew("sidebar");
	uiEntitySetSize(te,180,1);
	uiEntityFitY(te,1);
	uiEntityAlign(te,UI_ALIGN_EAST);
	te->margin_out = 2;
	
	t = uiRegionNew("Tool",180,1);
	uiEntityFitX(t,1);
	uiEntityFitY(t,1);
	uiTabAdd(t,te);



	t = uiRegionNew("Layers",180,1);
	uiEntityFitX(t,1);
	uiEntityFitY(t,1);
	uiTabAdd(t,te);

	t = uiRegionNew("History",180,1);
	uiEntityFitX(t,1);
	uiEntityFitY(t,1);
	uiTabAdd(t,te);

	return te;
}
int main(int argc, char **argv){
	uiEntity *s;
	uiNewWindow("Himalaya Alpha 1",800,600);
	s = uiScreenNew("MainScreen");
	uiScreenSet(s);	
	uiEntityAdd(uiSideBar(),s);
	uiMainLoop();
	return 0;
}
	
