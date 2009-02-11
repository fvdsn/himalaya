#include "uiCore.h"

typedef struct button_data{
	int id;
	void (*click)(uiEntity *self, int id);
}uiButtonData;

uiEntity *uiButtonNew(char *name,
		uiEntity *parent, 
		int id,
		void(*click)(uiEntity *self, int id));

typedef struct slider_data{
	float min_value;
	float max_value;
	float update_speed;
	float update_value;
	float value;
	float *dest_value;
	int id;
	void (*slide)(uiEntity*self, float value, int id);
}uiSliderData;

uiEntity *uiPanelNew(char *name, uiEntity *parent);

uiEntity *uiSliderNew(char *name,
		uiEntity *parent,
		int id,
		float min_value,
		float max_value,
		float update_speed,
		float *dest_value,
		void (*slide)(uiEntity*self,float value,int id));

uiEntity *uiColorNew(char *name,uiEntity *parent,float *color);

