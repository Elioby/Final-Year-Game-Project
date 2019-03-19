#include "board_eval.h"

#include "hashtable.h"
#include "map.h"
#include "team.h"

struct hashtable_los_result : hashtable_item
{
	bool has_los;
};

hashtable* los_cache_table;
hashtable_los_result* los_cache_data;

void board_eval_build_cache(team team)
{
	u32 total = entities.size();
	u32 enemies = 0;

	for (u32 i = 0; i < total; i++)
	{
		if (entities[i]->team != team)
		{
			enemies++;
		}
	}

	u32 friendlies = total - enemies;

	los_cache_table = hashtable_create(enemies * friendlies * 2);

	los_cache_data = (hashtable_los_result*) malloc(sizeof(hashtable_los_result) * enemies * friendlies);

	u32 index = 0;
	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* enemy = entities[i];
		if (enemy->team != team)
		{
			for (u32 j = 0; j < entities.size(); j++)
			{
				entity* friendly = entities[j];
				if (friendly->team == team)
				{
					hashtable_los_result* result = los_cache_data + index++;
					result->has_los = map_has_los(enemy, friendly);
					result->key = hashtable_hash_u32((u32) enemy->pos.x) + hashtable_hash_u32((u32) enemy->pos.z)
						+ hashtable_hash_u32((u32) friendly->pos.x) + hashtable_hash_u32((u32) friendly->pos.z);
					hashtable_put(los_cache_table, result);
				}
			}
		}
	}
}

void board_eval_destroy_cache()
{
	hashtable_destroy(los_cache_table);
	los_cache_table = NULL;

	free(los_cache_data);
}

float evaluate_cover(team team)
{
	float eval = 0.0f;

	// @Todo: abstract this out for use in UI code as well as AI code
	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* enemy = entities[i];
		if (enemy->team != team)
		{
			for (u32 j = 0; j < entities.size(); j++)
			{
				entity* friendly = entities[j];
				if (friendly->team == team)
				{
					bool has_los;

					// is the cache enabled?
					if (los_cache_table)
					{
						i32 hash = hashtable_hash_u32((u32) enemy->pos.x) + hashtable_hash_u32((u32) enemy->pos.z) 
							+ hashtable_hash_u32((u32) friendly->pos.x) + hashtable_hash_u32((u32) friendly->pos.z);
						hashtable_los_result* result = (hashtable_los_result*) hashtable_get(los_cache_table, hash);

						if (result != NULL)
						{
							has_los = result->has_los;
						}
						else
						{
							has_los = map_has_los(enemy, friendly);
						}
					}
					else
					{
						has_los = map_has_los(enemy, friendly);
					}

					if (has_los)
					{
						// calculate the chance our enemy can hit us and subtract from eval (it's our disadvantage)
						float enemy_hit_friendly = map_get_los_angle(enemy, friendly);
						eval -= enemy_hit_friendly;

						// calculate the chance we can hit our enemy and add it to our eval (it's our advantage)
						float friendly_hit_enemy = map_get_los_angle(friendly, enemy);
						eval += friendly_hit_enemy;
					}
				}
			}
		}
	}

	return eval;
}

float evaluate_health(team team)
{
	float eval = 0.0f;

	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* ent = entities[i];

		float entity_eval = (float) ent->health / ent->max_health;

		// @Consider: how important is having them alive or dead? id say pretty important, on top of how much HP they have (they'll get another action next turn)
		entity_eval += ent->dead ? -1.0f : 1.0f;

		// if they're on our team, its good for them to be alive, bad for dead, and good for more hp. the opposite is true if they're an enemy
		if (ent->team == team) eval += entity_eval;
		else eval -= entity_eval;
	}

	return eval;
}

float evaluate_board(team team)
{
	float eval = 0.0f;

	float cover = evaluate_cover(team);
	float health = evaluate_health(team);

	eval += cover + health;

	return eval;
}