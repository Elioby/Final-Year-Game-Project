#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_NO_FAILURE_STRINGS

// @Todo: should we malloc this?
image load_image(char* filename)
{
	int width, height, channels;

	unsigned char *pixels = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	if(pixels == 0)
	{
		printf("Failed to load image %s: \"%s\".\n", filename, stbi_failure_reason());
		return {};
	}

	return create_image(pixels, width, height, 4, BGFX_TEXTURE_FORMAT_RGBA8);
}

image create_image(void* pixels, u32 width, u32 height, u8 bytes_per_pixel, bgfx_texture_format_t pixel_format)
{
	image img = {};

	img.width = width;
	img.height = height;

	u32 data_byte_count = width * height * bytes_per_pixel;
	const bgfx_memory_t* mem = bgfx_make_ref(pixels, data_byte_count);
	img.handle = bgfx_create_texture_2d(width, height, 0, 1, pixel_format,
		BGFX_SAMPLER_POINT,
		mem);
	bgfx_make_ref_release(mem, data_byte_count, 0, 0);

	return img;
}