#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "uiCore.h"
#include "uiFont.h"

FT_Library library = NULL;	/* freetype library */
static void uiFontInit(){
	int error = 0;
	if(!library){
		error = FT_Init_FreeType(&library);
		if(error){
			printf("ERROR: could not init freetype !\n");
			exit(1);
		}
	}
}
uiFont* uiFontLoad(const char *name, int tex_size){
	uiFont *f = (uiFont*)malloc(sizeof(uiFont));
	FT_Face face;
	FT_GlyphSlot slot; 
	int error;
	int face_index = 0; 	/* the font index in the font file */
	int cindex = 256;	/* ascii index */
	float scale = 1.0f/(64.0f*tex_size); /* from 64th pixels to [0,1] */
	uiGlyph * g = NULL;
	uiFontInit();
	error = FT_New_Face(library, name, face_index, &face);
	if(error){
		printf("ERROR: could not load font : %s \n",name);
		free(f);
		return NULL;
	}
	
	strncpy(f->name,name,NAME_LENGTH);
	f->size = 1.0f;
	f->line_height = 2.0f;
	f->tab_to_space = 4.0f;
	f->space_width = 1.0f;

	slot = face->glyph;
	error = FT_Set_Pixel_Sizes(face,0,tex_size);
	
	while(cindex--){
		int px, x, y;
		g = &(f->glyph[cindex]);
		error = FT_Load_Char(face,cindex,FT_LOAD_RENDER);
		if(error){
			memset(g,0,sizeof(uiGlyph));
			printf("font:%s, glyph:%d ERROR\n",name,cindex);
			continue;
		}
		g->tex_sizex = slot->bitmap.width;
		g->tex_sizey = slot->bitmap.rows;
		g->ren_sizex = (float)g->tex_sizex/(float)tex_size;
		g->ren_sizey = (float)g->tex_sizey/(float)tex_size;
		printf("tsx:%d tsy:%d rsx:%f rsy:%f\n", g->tex_sizex,g->tex_sizey,g->ren_sizex,g->ren_sizey);

		g->advancex = scale * slot->advance.x;
		g->advancey = scale * slot->advance.y;
		g->bearx = scale * slot->bitmap_left;
		g->beary = scale * slot->bitmap_top;
		
		if(cindex == ' '){
			f->space_width = g->advancex;
		}
		px = g->tex_sizex*g->tex_sizey;
		g->tex = (char*)malloc(4*px*sizeof(char));
		memset(g->tex,255,4*px*sizeof(char));
		y = g->tex_sizey;
		while(y--){
			x = g->tex_sizex;
			while(x--){
				px = y*g->tex_sizex+x;
				g->tex[px*4 + 3] = ((char*)(slot->bitmap.buffer))[px];
			}
		}
		glGenTextures(1,&(g->gl_tex_id));
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB,g->gl_tex_id);
		/*glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		*/
		glTexImage2D(	GL_TEXTURE_RECTANGLE_ARB,
				0,
				GL_RGBA,
				g->tex_sizex,
				g->tex_sizey,
				0,
				GL_BGRA,
				GL_UNSIGNED_BYTE,
				g->tex );

	}
	return f;
}
static void uiGlyphDraw(float posx, float posy, float posz, float scale, uiGlyph *g){
	float x = posx + g->bearx*scale;
	float y = posy - g->beary*scale;
	float z = posz;
	float sx = g->ren_sizex*scale;
	float sy = g->ren_sizey*scale;
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB,g->gl_tex_id);
	/*printf("G:[%f,%f,%f]  [%f,%f] \n",x,y,z,sx,sy);*/
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f,(float)g->tex_sizey);
		glVertex3f(x,		y,		z);
		glTexCoord2f((float)g->tex_sizex,(float)g->tex_sizey);
		glVertex3f(x+sx,	y,		z);
		glTexCoord2f((float)g->tex_sizex,0.0f);
		glVertex3f(x+sx,	y+sy,		z);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(x,		y+sy,		z);
	glEnd();
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
}
void uiTextDraw(	float posx, float posy, float posz, float scale, 
			uiFont *font, const float *color, const char *string){
	int index = 0;
	float cposx = posx;
	float cposy = posy;
	uiGlyph *g = NULL;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if(color){
		glColor4f(color[0],color[1],color[2],color[3]);
	}
	while(string[index] != 0){
		g = &(font->glyph[(int)string[index]]);
		if(!g){
			cposx += font->space_width*scale;
		}else if( string[index] == '\n'){
			cposx = posx;
			cposy -= font->line_height*scale;
		}else if( string[index] == '\t'){
			cposx += font->space_width*font->tab_to_space*scale;
		}else if( string[index] == ' '){
			cposx += font->space_width*scale;
		}else if(string[index] != '\n'){
			uiGlyphDraw(cposx, cposy, posz, scale, g);
			cposx += g->advancex*scale;
			cposy += g->advancey*scale;
		}
		index++;
	}
}
