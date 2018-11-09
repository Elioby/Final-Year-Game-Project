#include "shader.h"

#include <stdio.h>

/* ------- Globals --------- */
shader_program default_shader;
shader_program diffuse_shader;

void load_shaders()
{
	diffuse_shader = load_shader_program("res/vs_cubes.bin", "res/fs_cubes.bin");
	default_shader = load_shader_program("res/default_vs.bin", "res/default_fs.bin");
}

// Load an individual vertex or fragment shader from a file path
bgfx_shader_handle_t load_shader(char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "rb");

	if (file == NULL)
	{
		printf("Failed to load shader from file %s\n", filename);

		// @Safety
		return{};
	}

	// @Cleanup: if we do a lot of file io we could make a wrapper for this stuff?
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	void* shader_data = malloc(file_size + 1);
	fread(shader_data, file_size, 1, file);
	((char*)shader_data)[file_size] = '\0';

	bgfx_shader_handle_t shader = bgfx_create_shader(bgfx_make_ref(shader_data, file_size + 1));

	// We're done with the data and file now
	fclose(file);

	// @Todo: bgfx_set_shader_name(shader, filename, strlen(filename));
	return shader;
}

// @Todo: (for this and load model) use names instead of paths? i.e. shader vs_cube resolves to res/shaders/dx11/vs_cube.bin and model cube resolves to res/models/cube.obj
// @Todo: add support for other renderers with different shaders for each type? it's not hard, just different folders for each shader type, compile with shaderc, donezo
shader_program load_shader_program(char* vs_filename, char* fs_filename)
{
	if (renderer_type != BGFX_RENDERER_TYPE_OPENGL)
	{
		printf("ogl is currently the only supported renderer :(\n");
		// @Safety: we should always return SOMETHING, not something blank! (default shader program?)
		return{};
	}

	bgfx_shader_handle_t vs_handle = load_shader(vs_filename);
	bgfx_shader_handle_t fs_handle = load_shader(fs_filename);

	shader_program shader;
	shader.handle = bgfx_create_program(vs_handle, fs_handle, true);

	if (shader.handle.idx == UINT16_MAX)
	{
		printf("Failed to create program from shaders %s and %s\n", vs_filename, fs_filename);
		// @Safety
		return{};
	}

	return shader;
}