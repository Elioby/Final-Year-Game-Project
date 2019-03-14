#pragma once

#include "general.h"
#include "graphics.h"

struct shader_program
{
	bgfx_program_handle_t handle;
};

shader_program load_shader_program(char* vs_filename, char* fs_filename);