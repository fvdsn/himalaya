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

uiEntity *uiPanelNew(char *name);

uiEntity *uiSliderNew(char *name,
		int id,
		float min_value,
		float max_value,
		float update_speed,
		float *dest_value,
		void (*slide)(uiEntity*self,float value,int id));

uiEntity *uiColorNew(char *name,float *color);

