#include "font.h"

#include "file.h"

#include <stdio.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

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