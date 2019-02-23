#ifndef MESH_H
#define MESH_H

#include "general.h"

struct pos_normal_vertex
{
	float x, y, z;
	float u, v;
	u32 normal;
};

struct mesh
{
	pos_normal_vertex* vertices;
	bgfx_vertex_buffer_handle_t vb_handle;
	u32 vertex_count;

	u16* indices;
	bgfx_index_buffer_handle_t idb_handle;
	u32 index_count;
};

inline u32 pack_vec3_into_u32(vec3 vec);

mesh mesh_create(pos_normal_vertex* vertices, u32 vertex_count);
mesh mesh_create(pos_normal_vertex* vertices, u32 vertex_count, u16* indices, u32 index_count);

void mesh_destroy(mesh m);

mesh load_obj_mesh(char* filename);

#endif