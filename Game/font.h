#pragma once

#include "image.h"

#include "stb_truetype.h"

struct font {
	image img;
	stbtt_bakedchar* char_data;
	u32 width, height;
};

font* load_font(char* filename);
