#pragma once

#include "general.h"
#include "mesh.h"

#include <vector>

struct entity {
	u32 id;
	vec3 pos;

	mesh mesh;

	i32 health;
	i32 max_health;

	i32 ap;
	i32 max_ap;

	bool enemy;
	bool dead;
};

extern std::vector<entity*> entities;

// @Cleanup: honestly idk where else to put this
extern entity* selected_entity;

void entity_update();

void entity_add(vec3 pos, bool enemy);
entity* entity_get_at_block(vec3 block_pos);

void entity_health_change(entity* ent, entity* inflict_ent, i32 amount);
bool entity_is_same_team(entity* ent1, entity* ent2);