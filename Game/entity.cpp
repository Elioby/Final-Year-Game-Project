#include "entity.h"

#include "assets.h"
#include "map.h"

// @Todo: change with a quadtree!
std::vector<entity*> entities;

void entity_add(vec3 pos, bool enemy)
{
	// @Todo: use of malloc :(
	entity* ent = (entity*) malloc(sizeof(entity));
	ent->pos = pos;
	ent->mesh = robot_mesh;
	ent->max_health = 10;
	ent->health = ent->max_health;
	ent->dead = false;
	ent->enemy = enemy;
	entities.push_back(ent);
}

vec3 entity_get_block_pos(entity entity)
{
	return map_get_block_pos(entity.pos);
}

// @Todo: move to map??????
entity* entity_get_at_block(vec3 block_pos)
{
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];
		if(map_pos_equal(entity->pos, block_pos))
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