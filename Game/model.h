#ifndef MODEL_H
#define MODEL_H

#include "general.h"

struct pos_normal_vertex
{
	float x, y, z;
	u32 normal;
};

struct model
{
	pos_normal_vertex* vertices;
	bgfx_vertex_buffer_handle_t vb_handle;
	u32 vertex_count;

	u16* indices;
	bgfx_index_buffer_handle_t vidb_handle;
	u32 vertex_index_count;
};

model load_obj_model(char* filename);

#endif