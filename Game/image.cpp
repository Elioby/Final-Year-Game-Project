#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_NO_FAILURE_STRINGS

image load_image(char* filename)
{
	image img = {};

	int width, height, channels;

	unsigned char *data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	if(data == 0)
	{
		printf("Failed to load image %s: \"%s\".\n", filename, stbi_failure_reason());
		return {};
	}

	img.width = width;
	img.height = height;

	u32 data_byte_count = width * height * 4;
	const bgfx_memory_t* mem = bgfx_make_ref(data, data_byte_count);
	img.handle = bgfx_create_texture_2d(width, height, 0, 1, BGFX_TEXTURE_FORMAT_RGBA8, 
		BGFX_SAMPLER_POINT,
		mem);
	bgfx_make_ref_release(mem, data_byte_count, 0, 0);

	u32 in = (BGFX_SAMPLER_POINT & BGFX_SAMPLER_MAG_MASK) >> BGFX_SAMPLER_MAG_SHIFT;
	u32 in2 = (BGFX_SAMPLER_POINT & BGFX_SAMPLER_MIN_MASK) >> BGFX_SAMPLER_MIN_SHIFT;

	return img;
}