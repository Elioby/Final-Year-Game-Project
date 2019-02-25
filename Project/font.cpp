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

u32 font_get_text_width(font* font, char* text, u16 text_len, float scale)
{
	u32 i = 0;

	float x1 = 0;
	float y1 = 0;

	float total_width = 0.0f;

	stbtt_bakedchar *baked_start = font->char_data - 32;

	while (true)
	{
		char c = text[i++];

		if (c == 0) break;
		stbtt_bakedchar* b = baked_start + c;
		total_width += b->xadvance * scale;
	}

	return (u32) total_width;
}

u32 font_get_text_width(font* font, char* text, float scale)
{
	return font_get_text_width(font, text, strlen(text), scale);
}

u32 font_get_text_width(font* font, dynstr* text, float scale)
{
	return font_get_text_width(font, text->raw, text->len, scale);
}