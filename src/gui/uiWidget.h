#include "uiCore.h"

#define UI_TEXT_LENGTH 1024

typedef struct button_data{
	int id;
	void (*click)(uiEntity *self, int id);
}uiButtonData;

typedef struct label_data{
	char text[UI_TEXT_LENGTH];
	float font_color[4];
	int   font_police;
	int   font_style;
	float   font_size;
}uiLabelData;

uiEntity *uiScreenNew(char *name);
	
uiEntity *uiButtonNew(char *name,
		int id,
		void(*click)(uiEntity *self, int id));
uiEntity *uiLabelNew(char *name, 
		const char *text, 
		const float *col,
		int font_police,
		int font_style,
		float font_size);
uiEntity *uiCairoLabelNew(char *name, 
		const char *text,
		const float *col,
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


uiEntity *uiSliderNew(char *name,
		int id,
		float min_value,
		float max_value,
		float update_speed,
		float *dest_value,
		void (*slide)(uiEntity*self,float value,int id));

uiEntity *uiPanelNew(char *name);
uiEntity *uiColorNew(char *name,float *color);

uiEntity *uiTabEnvNew(char *name);
uiEntity *uiTabNew(char *name);
void uiTabAdd(uiEntity *tabenv, uiEntity *tab);

typedef struct region_data{
	float inner_sizex;
	float inner_sizey;
	float dx;
	float dy;
}uiRegionData;

uiEntity *uiRegionNew(char *name,float inner_sx, float inner_sy);
uiEntity *uiRectNew(char *name,float sx, float sy, float r, float g, float b);
