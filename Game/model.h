#ifndef MODEL_H
#define MODEL_H

#include "general.h"

struct model
{
	bgfx_vertex_buffer_handle_t vb_handle;
	u32 vertex_count;

	bgfx_index_buffer_handle_t vidb_handle;
	u32 vertex_index_count;
};

struct pos_normal_vertex
{
	float x, y, z;
	u32 normal;
};

model load_obj_model(char* filename);

#endif