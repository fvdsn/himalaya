#ifndef __UI_WIDGET_H__
#define __UI_WIDGET_H__

#include "uiCore.h"

#define UI_TEXT_LENGTH 1024
#define UI_BUTTON_WIDTH 60
#define UI_BUTTON_HEIGHT 16
#define UI_PANEL_WIDTH 150
#define UI_PANEL_HEIGHT 120

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
	float min_value;	/*minimum value of the slider */
	float max_value;	/*maximum value of the slider */
	float update_speed;	/*how much the value increases by dragged pixel */
	float value;		/*the value of the slider */
	float base;		/*if the slider is exponential, the base of the exponent */
	float exp_value;	/* = base^value. is in [min_value,max_value] */
	float *dest_value;
	int id;
	void (*slide)(uiEntity*self, float value, int id);
}uiSliderData;

uiEntity *uiSliderNew(const char *name,
		int id,
		float min_value,
		float max_value,
		float update_speed,
		float base,
		float start_value,
		float *dest_value,
		void (*slide)(uiEntity*self,float value,int id));
void	 uiSliderSetValue(uiEntity *slider, float value);
float	 uiSliderGetValue(uiEntity *slider);

uiEntity *uiDisplayFloatNew(const char *name, float *display_value);
uiEntity *uiDisplayIntNew(const char *name, int *display_value);

uiEntity *uiPanelNew(const char *name);
uiEntity *uiColorNew(const char *name,float *color);

uiEntity *uiTabEnvNew(const char *name);
int uiTabAdd(uiEntity *tabenv, uiEntity *tab);

uiEntity *uiRegionNew(const char *name,float inner_sx, float inner_sy);
uiEntity *uiRectNew(const char *name,float sx, float sy, float r, float g, float b);
void uiRectDraw(float x, float y, float z,float sx, float sy);

uiEntity *uiDivNew(const char *name, float sx, float sy);
#endif

