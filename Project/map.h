#pragma once

#include <vector>

#include "general.h"
#include "mesh.h"
#include "entity.h"

// the step of the world raytrace
#define MAP_RAYTRACE_ACCURACY 0.1f

extern u32 terrain_max_x;
extern u32 terrain_max_z;

struct cover {
	vec3 pos;

	u32 height;
};

extern std::vector<cover*> cover_list;
extern std::vector<vec3> map_debug_los;

void map_init();

mesh* map_generate_terrain_mesh(); 

vec3 map_get_block_pos(vec3 pos);
bool map_pos_equal(vec3 pos1, vec3 pos2);
float map_distance_squared(vec3 pos1, vec3 pos2);

void map_add_cover(vec3 pos);
cover* map_get_cover_at_block(vec3 block_pos);
entity* map_get_entity_at_block(vec3 block_pos);

cover* map_get_adjacent_cover(vec3 start, vec3 closest_to);

bool map_has_los(entity* ent1, entity* ent2);

// Returns a floating point value between 0.0f and 1.0f (0.0f = 100% covered, 0.5f = 50% covered, 1.0f = no blocking cover)
float map_get_los_angle(entity* inflict_ent, entity* target_ent);