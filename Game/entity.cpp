#include "entity.h"

#include "assets.h"

// @Todo: change with a quadtree!
std::vector<entity> entities;

void add_default_entity(vec3 pos)
{
	entity ent = {};
	ent.pos = pos;
	ent.mesh = robot_mesh;
	ent.health = 100;
	ent.max_health = 100;
	entities.push_back(ent);
}