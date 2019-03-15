#pragma once

#include "general.h"
#include "graphics.h"
#include "asset.h"

struct shader_program : asset
{
	bgfx_program_handle_t handle;
};

void shader_init();
void shader_set_tint_uniform(vec4 color);
shader_program* load_shader_program(char* asset_id, char* vs_filename, char* fs_filename);