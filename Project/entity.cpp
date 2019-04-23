#include "entity.h"

#include "asset_manager.h"
#include "map.h"
#include "actionbar.h"

dynarray* entities = dynarray_create(10, sizeof(entity));

entity* selected_entity;
u32 last_entity_id;

void entity_update()
{
	// if our unit dies on our turn, unselect them!
	if (selected_entity != NULL && selected_entity->dead) selected_entity = NULL;

	for (u32 i = 0; i < entities->len; i++)
	{
		entity* ent = (entity*) dynarray_get(entities, i);

		if (ent->dead) 
		{
			dynarray_remove(entities, i);
			i--;
		}
	}
}

void entity_add(vec3 pos, team team)
{
	entity ent = { 0 };
	ent.id = last_entity_id++;
	ent.pos = pos;
	ent.mesh = asset_manager_get_mesh("robot");
	ent.max_health = 10;
	ent.health = ent.max_health;
	ent.max_ap = 100;
	ent.ap = 0;
	ent.dead = false;
	ent.team = team;
	dynarray_add(entities, &ent);
}

void entity_kill(entity* ent, bool temp)
{
	ent->dead = true;
	ent->health = 0;

	// @Todo: what if temp?
	if(!temp)
	{

	}
}

void entity_kill(entity* ent)
{
	entity_kill(ent, false);
}

void entity_health_change(entity* target_ent, entity* inflict_ent, s32 amount)
{
	entity_health_change(target_ent, inflict_ent, amount, false);
}

void entity_health_change(entity* target_ent, entity* inflict_ent, s32 amount, bool temp)
{
	debug_assert(target_ent && inflict_ent && !inflict_ent->dead, "A valid target and inflict entity must be provided");
	debug_assert(!target_ent->dead || temp, "The target entity cannot be dead (unless this is a temporary health change for eval)");

	target_ent->health += amount;

	if(!temp)
	{
		if (amount > 0) actionbar_combatlog_add("Entity %i healed by entity %i by %i", inflict_ent->id, target_ent->id, amount);
		else actionbar_combatlog_add("Entity %i did %i dmg to entity %i", inflict_ent->id, -amount, target_ent->id);
	}

	if(target_ent->health <= 0)
	{
		entity_kill(target_ent);

		if(!temp)
		{
			actionbar_combatlog_add("Entity %i killed entity %i!", inflict_ent->id, target_ent->id);
		}
	}
	else if(target_ent->health > 0)
	{
		if(target_ent->health > target_ent->max_health) target_ent->health = target_ent->max_health;

		target_ent->dead = false;
	}
}

bool entity_is_same_team(entity* ent1, entity* ent2)
{
	return ent1->team == ent2->team;
}