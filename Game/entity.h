#pragma once

#include "general.h"
#include "mesh.h"

#include <vector>

struct entity {
	vec3 pos;

	mesh mesh;

	i32 health;
	i32 max_health;

	bool dead;
};

extern std::vector<entity*> entities;

void entity_add(vec3 pos);
entity* entity_get_at_block(vec3 block_pos);

void entity_health_change(entity* entity, u32 amount);