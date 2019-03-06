#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_NO_FAILURE_STRINGS

#include "file.h"
#include "dynstr.h"

// @Todo: should we malloc this?
image* load_image(char* path)
{
	int width, height, channels;

	// @Todo: use our file load function
	unsigned char *pixels = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);

	if(pixels == 0)
	{
		printf("Failed to load image %s: \"%s\".\n", path, stbi_failure_reason());
		return {};
	}

	u32 len = file_get_length_without_extension(path);
	u32 filename_start = file_get_filename_start(path);

	dynstr* asset_id = dynstr_new(path, len);
	dynstr_trim_start(asset_id, filename_start);
	

	image* img = create_image(asset_id->raw, pixels, width, height, 4, BGFX_TEXTURE_FORMAT_RGBA8);

	dynstr_free(asset_id);

	return img;
}

image* create_image(image* img_memory, void* pixels, u32 width, u32 height, u8 bytes_per_pixel, bgfx_texture_format_t pixel_format)
{
	img_memory->asset_id = NULL;
	img_memory->asset_type = ASSET_TYPE_UNTRACKED;

	img_memory->width = width;
	img_memory->height = height;

	u32 data_byte_count = width * height * bytes_per_pixel;
	const bgfx_memory_t* mem = bgfx_make_ref(pixels, data_byte_count);
	img_memory->handle = bgfx_create_texture_2d(width, height, 0, 1, pixel_format,
		BGFX_SAMPLER_POINT,
		mem);
	bgfx_make_ref_release(mem, data_byte_count, 0, 0);

	return img_memory;
}

image* create_image(void* pixels, u32 width, u32 height, u8 bytes_per_pixel, bgfx_texture_format_t pixel_format)
{
	image* img = (image*) malloc(sizeof(image));
	return create_image(img, pixels, width, height, bytes_per_pixel, pixel_format);
}

// Note that asset_id is copied into the asset struct, so you can free it after the function returns
image* create_image(char* asset_id, void* pixels, u32 width, u32 height, u8 bytes_per_pixel, bgfx_texture_format_t pixel_format)
{
	u32 asid_len = strlen(asset_id) + 1;
	image* img = (image*) malloc(sizeof(image) + asid_len);

	create_image(img, pixels, width, height, bytes_per_pixel, pixel_format);

	// copy the asset_id to after the struct
	img->asset_id = ((char*) img) + sizeof(image);
	img->asset_type = ASSET_TYPE_IMAGE;

	memcpy(img->asset_id, asset_id, asid_len);

	return img;
}