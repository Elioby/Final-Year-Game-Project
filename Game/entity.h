#pragma once

#include "general.h"
#include "mesh.h"

#include <vector>

struct entity {
	vec3 pos;

	mesh mesh;

	u32 health;
	u32 max_health;
};

extern std::vector<entity> entities;

void add_default_entity(vec3 pos);