#pragma once

#include "general.h"

struct image
{
	u32 width;
	u32 height;
	bgfx_texture_handle_t handle;
};

image load_image(char* filename);