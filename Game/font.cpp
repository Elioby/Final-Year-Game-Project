#include "font.h"

#include "file.h"

#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "graphics.h"

font* load_font(char* filename)
{
	file_data* file = load_file(filename);

	if(!file)
	{
		printf("Failed to load font \"%s\"", filename);
	}

	const u32 char_count = 96;

	// @Todo: use only one malloc!
	font* fnt = (font*) malloc(sizeof(font) + sizeof(stbtt_bakedchar) * char_count);

	fnt->width = 1024;
	fnt->height = 1024;

	unsigned char* pixels = (unsigned char*) malloc(fnt->width * fnt->height);
	fnt->char_data = (stbtt_bakedchar*) (((char*) fnt) + sizeof(font));

	unsigned char* font_data = (unsigned char*) file->data;

	stbtt_BakeFontBitmap(font_data, 0, 128.0f, pixels, fnt->width, fnt->height, 32, char_count, fnt->char_data);

	fnt->img = create_image(pixels, fnt->width, fnt->height, 1, BGFX_TEXTURE_FORMAT_A8);

	return fnt;
}


u32 font_get_text_width(font* font, char* text, float scale)
{
	u32 i = 0;

	scale *= 2.0f;

	float x1 = 0;
	float y1 = 0;

	float total_width = 0.0f;

	while(true)
	{
		char c = text[i++];

		if(c == 0) break;

		const stbtt_bakedchar *b = font->char_data + c - 32;
		stbtt_aligned_quad q = {};
		stbtt_GetBakedQuad(font->char_data, font->width, font->height, c - 32, &x1, &y1, &q, 1);// @Volatile: 1=opengl & d3d10+,0=d3d9

		total_width += (x1 / (20.0f / scale)) + (q.x1 * scale) - (q.x0 * scale);
	}

	return (u32) total_width / 2.0f;
}