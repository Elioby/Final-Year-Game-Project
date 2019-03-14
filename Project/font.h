#pragma once

#include "image.h"
#include "dynstr.h"
#include "asset.h"

#include "stb_truetype.h"

struct font : asset
{
	image* img;
	stbtt_bakedchar* char_data;
	u32 width, height;
};

font* load_font(char* asset_type, char* filename);

u32 font_get_text_width(font* font, char* text, float scale);
u32 font_get_text_width(font* font, dynstr* text, float scale);