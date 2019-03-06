#pragma once

#include "graphics.h"
#include "asset.h"

struct shader_program : asset
{
	bgfx_program_handle_t handle;
};

shader_program* load_shader_program(char* asset_id, char* vs_filename, char* fs_filename);