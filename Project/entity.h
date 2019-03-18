#pragma once

#include "general.h"
#include "mesh.h"
#include "team.h"

#include <vector>

struct entity {
	u32 id;
	vec3 pos;

	mesh* mesh;

	i32 health;
	i32 max_health;

	i32 ap;
	i32 max_ap;

	team team;
	bool dead;
};

extern std::vector<entity*> entities;

// @Cleanup: honestly idk where else to put this
extern entity* selected_entity;

void entity_update();

void entity_add(vec3 pos, team t);

void entity_health_change(entity* ent, entity* inflict_ent, i32 amount);
void entity_health_change(entity* ent, entity* inflict_ent, i32 amount, bool temp);
bool entity_is_same_team(entity* ent1, entity* ent2);