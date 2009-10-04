#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cairo.h>
#include "uiCore.h"

uiString *uiStringNew(const char *text, float font_size, int sx, int sy, float px, float py){
	uiString *s = (uiString*)malloc(sizeof(uiString));
	memset(s,0,sizeof(uiString));
	strncpy(s->text,text,UI_STRING_LENGTH);
	s->font = "sans";
	s->color[3] = 1;
	s->font_size = font_size;
	s->font_weight = CAIRO_FONT_WEIGHT_NORMAL;
	s->font_slant  = CAIRO_FONT_SLANT_NORMAL;
	s->px = px;
	s->py = py;
	s->sx = sx;
	s->sy = sy;
	s->uptodate = 0;
	return s;
}
void uiStringDraw(uiString *s, float x, float y, float z, float sizex, float sizey){
	cairo_surface_t *surface;
	cairo_t *cr;
	int sx = (int)sizex;
	int sy = (int)sizey;
	if(!s->uptodate || sx > s->sx || sy > s->sy || !s->buffer){
		printf("rendering buffer\n");
		free(s->buffer);
		s->buffer = malloc(sx*sy*4);
		memset(s->buffer,0,sx*sy*4);
		s->sx = sx;
		s->sy = sy;
		surface = cairo_image_surface_create_for_data((unsigned char*)s->buffer,
				CAIRO_FORMAT_ARGB32,
				sx,
				sy,
				4*sx);
		cr = cairo_create(surface);
		cairo_select_font_face(cr,s->font,s->font_slant,s->font_weight);
		cairo_set_font_size(cr,s->font_size);
		cairo_set_source_rgb(cr,s->color[0],s->color[1],s->color[2]);
		cairo_move_to(cr,s->px,s->py);
		cairo_show_text(cr,s->text);
		cairo_destroy(cr);
		cairo_surface_finish(surface);
		s->uptodate = 1;
	}
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glTexImage2D(	GL_TEXTURE_RECTANGLE_ARB,
			0,
			GL_RGBA,
			s->sx,
			s->sy,
			0,
			GL_BGRA,
			GL_UNSIGNED_BYTE,
			s->buffer );
	glBegin(GL_QUADS);
		glTexCoord2f(0.0,sy);
		glVertex3f(x,		y,		z);
		glTexCoord2f(sx,sy);
		glVertex3f(x+sx,	y,		z);
		glTexCoord2f(sx,0.0);
		glVertex3f(x+sx,	y+sy,		z);
		glTexCoord2f(0.0,0.0);
		glVertex3f(x,		y+sy,		z);
	glEnd();
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
}
