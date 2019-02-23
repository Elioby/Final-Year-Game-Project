#pragma once

#include "general.h"

struct image
{
	u32 width;
	u32 height;
	bgfx_texture_handle_t handle;
};

image load_image(char* filename);
image create_image(void* pixels, u32 width, u32 height, u8 bytes_per_pixel, bgfx_texture_format_t pixel_format);