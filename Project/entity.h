#pragma once

#include "general.h"
#include "mesh.h"

#include <vector>

typedef enum team
{
	TEAM_FRIENDLY,
	TEAM_ENEMY
} team_t;

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

void entity_add(vec3 pos, team team);

void entity_health_change(entity* ent, entity* inflict_ent, i32 amount);
bool entity_is_same_team(entity* ent1, entity* ent2);