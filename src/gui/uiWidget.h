#ifndef __UI_WIDGET_H__
#define __UI_WIDGET_H__

#include "uiCore.h"

#define UI_TEXT_LENGTH 1024

typedef struct button_data{
	int id;
	void (*click)(uiEntity *self, int id);
}uiButtonData;

uiEntity *uiScreenNew(const char *name);
	
uiEntity *uiButtonNew(const char *name,
		int id,
		void(*click)(uiEntity *self, int id));
uiEntity *uiLabelNew(const char *name, 
		const char *text, 
		float font_size);

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

uiEntity *uiSliderNew(const char *name,
		int id,
		float min_value,
		float max_value,
		float update_speed,
		float *dest_value,
		void (*slide)(uiEntity*self,float value,int id));

uiEntity *uiPanelNew(const char *name);
uiEntity *uiColorNew(const char *name,float *color);

uiEntity *uiTabEnvNew(const char *name);
int uiTabAdd(uiEntity *tabenv, uiEntity *tab);

uiEntity *uiRegionNew(const char *name,float inner_sx, float inner_sy);
uiEntity *uiRectNew(const char *name,float sx, float sy, float r, float g, float b);
void uiRectDraw(float x, float y, float z,float sx, float sy);

uiEntity *uiDivNew(const char *name, float sx, float sy);
#endif

