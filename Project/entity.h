#pragma once

#include "general.h"
#include "mesh.h"
#include "team.h"
#include "dynarray.h"

#include <vector>

struct entity {
	u32 id;
	vec3 pos;

	mesh* mesh;

	s32 health;
	s32 max_health;

	s32 ap;
	s32 max_ap;

	team team;
	bool dead;
};

extern dynarray* entities;

// @Cleanup: honestly idk where else to put this
extern entity* selected_entity;

void entity_update();

void entity_add(vec3 pos, team t);

void entity_health_change(entity* ent, entity* inflict_ent, s32 amount);
void entity_health_change(entity* ent, entity* inflict_ent, s32 amount, bool temp);
bool entity_is_same_team(entity* ent1, entity* ent2);