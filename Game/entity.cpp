#include "entity.h"

#include "assets.h"

// @Todo: change with a quadtree!
std::vector<entity*> entities;

void entity_add(vec3 pos)
{
	// @Todo: use of malloc :(
	entity* ent = (entity*) malloc(sizeof(entity));
	ent->pos = pos;
	ent->mesh = robot_mesh;
	ent->max_health = 10;
	ent->health = ent->max_health;
	ent->dead = false;
	entities.push_back(ent);
}

vec3 entity_get_block_pos(entity entity)
{
	return vec3(floor(entity.pos.x), floor(entity.pos.y), floor(entity.pos.z));
}

entity* entity_get_at_block(vec3 block_pos)
{
	for(int i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];
		glm::vec<3, bool> vec = glm::equal(block_pos, entity_get_block_pos(*entity));
		if(vec.x && vec.z)
		{
			return entity;
		}
	}

	return 0;
}

void entity_health_change(entity* entity, u32 amount)
{
	entity->health += amount;

	if(entity->health <= 0)
	{
		entity->dead = true;
		entity->health = 0;
	}

}