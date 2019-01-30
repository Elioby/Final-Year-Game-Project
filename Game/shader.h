#pragma once

#include "graphics.h"

struct shader_program
{
	bgfx_program_handle_t handle;
};

extern shader_program default_shader;
extern shader_program diffuse_shader;
extern shader_program gui_shader;

shader_program load_shader_program(char* vs_filename, char* fs_filename);
void load_shaders();