#pragma once

#include <vector>

#include "general.h"
#include "mesh.h"

extern u32 terrain_max_x;
extern u32 terrain_max_z;

struct cover {
	vec3 pos;

	u32 height;
};

extern std::vector<cover*> cover_list;

void map_init();
mesh load_terrain_mesh();
vec3 map_get_block_pos(vec3 pos);
bool map_pos_equal(vec3 pos1, vec3 pos2);
cover* map_get_cover_at_block(vec3 block_pos);