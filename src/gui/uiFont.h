#ifndef __UI_FONT_H__
#define __UI_FONT_H__

#define ASCII_COUNT 256
#define STRING_LENGTH 1024
#define NAME_LENGTH 128
#define DEFAULT_FONT_SIZE 24

typedef struct ui_glyph{
	int   tex_sizex;	/*size of the texture in pixels*/
	int   tex_sizey;	/*size of the texture in pixels*/
	float ren_sizex;	/*size of the texture render on screen (pixels) */
	float ren_sizey;	/*size of the texture render on screen (pixels) */
	float advancex;	/*position of the next glyph*/
	float advancey;	/*position of the next glyph*/
	float bearx;	/*relative x position of this glyph*/
	float beary;	/*relative y position of this glyph*/
	char *tex;
	unsigned int gl_tex_id;
}uiGlyph;

typedef struct ui_font{
	char name[NAME_LENGTH];
	float size;
	float line_height;
	float space_width;
	float tab_to_space;
	uiGlyph glyph[ASCII_COUNT];
}uiFont;

uiFont* uiFontLoad(const char *name, int tex_size);
void uiTextDraw(	float posx, float posy, float posz, float scale, 
			uiFont *font, const float * color, const char *string );


#endif
