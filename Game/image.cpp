#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

image load_image(char* filename)
{
	image img = {};

	int width, height, channels;

	unsigned char *data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	img.width = width;
	img.height = height;

	u32 data_byte_count = width * height * 4;
	const bgfx_memory_t* mem = bgfx_make_ref(data, data_byte_count);
	img.handle = bgfx_create_texture_2d(width, height, 0, 1, BGFX_TEXTURE_FORMAT_RGBA8, 0, mem);
	bgfx_make_ref_release(mem, data_byte_count, 0, 0);

	return img;
}