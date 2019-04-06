#ifndef MESH_H
#define MESH_H

#include "general.h"
#include "asset.h"

struct pos_normal_vertex
{
	float x, y, z;
	float u, v;
	u32 normal;
};

struct mesh : asset
{
	pos_normal_vertex* vertices;
	bgfx_vertex_buffer_handle_t vb_handle;
	u32 vertex_count;

	u32* indices;
	bgfx_index_buffer_handle_t idb_handle;
	u32 index_count;
};

inline u32 pack_vec3_into_u32(vec3 vec);

mesh* mesh_create(char* asset_id, pos_normal_vertex* vertices, u32 vertex_count, u32* indices, u32 index_count);
void mesh_destroy(mesh* m);

mesh* load_obj_mesh(char* asset_id, char* filename);

#endif