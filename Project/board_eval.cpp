#include "board_eval.h"

#include "hashtable.h"
#include "map.h"
#include "team.h"

hashtable* los_cache_table;

void board_eval_build_cache()
{
	team team = TEAM_ENEMY;

	u32 total = entities->len;
	u32 enemies = 0;

	for (u32 i = 0; i < total; i++)
	{
		if ((*((entity**) dynarray_get(entities, i)))->team != team)
		{
			enemies++;
		}
	}

	u32 friendlies = total - enemies;

	los_cache_table = hashtable_create(enemies * friendlies * 10, sizeof(bool));

	u32 index = 0;
	for (u32 i = 0; i < entities->len; i++)
	{
		entity* enemy = *((entity**) dynarray_get(entities, i));
		if (enemy->team != team)
		{
			for (u32 j = 0; j < entities->len; j++)
			{
				entity* friendly = *((entity**) dynarray_get(entities, i));
				if (friendly->team == team)
				{
					bool result = map_has_los(enemy, friendly);
					s32 hash = hashtable_hash_u32((u32) enemy->pos.x) + hashtable_hash_u32((u32) enemy->pos.z)
						+ hashtable_hash_u32((u32) friendly->pos.x) + hashtable_hash_u32((u32) friendly->pos.z);
					hashtable_put(los_cache_table, hash, &result);
				}
			}
		}
	}
}

void board_eval_destroy_cache()
{
	hashtable_destroy(los_cache_table);
	los_cache_table = NULL;
}

// maybe its okay to los the enemy if we have enough ap to kill someone? if not, stay in cover?
// if we're low on health, we should be out of los?
float evaluate_shot_chance(team team)
{
	float eval = 0.0f;

	float friendly_hit_enemy_weight = 4.0f;
	float enemy_hit_friendly_weight = -5.0f;

	// @Todo: abstract this out for use in UI code as well as AI code
	for (u32 i = 0; i < entities->len; i++)
	{
		entity* enemy = *((entity**) dynarray_get(entities, i));
		if (!enemy->dead && enemy->team != team)
		{
			for (u32 j = 0; j < entities->len; j++)
			{
				entity* friendly = *((entity**) dynarray_get(entities, j));
				if (!friendly->dead && friendly->team == team)
				{
					bool has_los;

					// is the cache enabled?
					if (los_cache_table)
					{
						s32 hash = hashtable_hash_u32((u32) enemy->pos.x) + hashtable_hash_u32((u32) enemy->pos.z) 
							+ hashtable_hash_u32((u32) friendly->pos.x) + hashtable_hash_u32((u32) friendly->pos.z);
						bool* result = (bool*) hashtable_get(los_cache_table, hash);

						if (result != NULL)
						{
							has_los = *result;
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
						float enemy_hit_friendly = enemy_hit_friendly_weight * map_get_shot_chance(enemy, friendly);
						eval += enemy_hit_friendly;

						// calculate the chance we can hit our enemy and add it to our eval (it's our advantage)
						float friendly_hit_enemy = friendly_hit_enemy_weight * map_get_shot_chance(friendly, enemy);
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

	float health_friendly_weight = 1.0f;
	float health_enemy_weight = -2.0f;

	float dead_friendly_weight = -1.0f;
	float dead_enemy_weight = 3.0f;

	float alive_friendly_weight = 0.0f;
	float alive_enemy_weight = 0.0f;

	for (u32 i = 0; i < entities->len; i++)
	{
		entity* ent = *((entity**) dynarray_get(entities, i));

		float entity_eval = (float) ent->health / ent->max_health;

		// if they're on our team, its good for them to be alive, bad for dead, and good for more hp. the opposite is true if they're an enemy
		if (ent->team == team)
		{
			eval += health_friendly_weight * entity_eval;

			// @Consider: how important is having them alive or dead? id say pretty important, on top of how much HP they have (they'll get another action next turn)
			eval += ent->dead ? dead_friendly_weight : alive_friendly_weight;
		}
		else
		{
			eval += health_enemy_weight * entity_eval;

			// @Consider: how important is having them alive or dead? id say pretty important, on top of how much HP they have (they'll get another action next turn)
			eval += ent->dead ? dead_enemy_weight : alive_enemy_weight;
		}
	}

	return eval;
}

#define DISTANCE_CUTOFF 10

float evaluate_distance_to_enemy(team team)
{
	float eval = 0.0f;

	// @Weight: the further away you are (squared) the worse
	float per_distance_sq_weight = -0.0001f;

	for (u32 i = 0; i < entities->len; i++)
	{
		entity* friendly = *((entity**) dynarray_get(entities, i));
		if (!friendly->dead && friendly->team == team)
		{
			bool anyone_alive = false;
			float smallest_distance = FLT_MAX;

			for (u32 j = 0; j < entities->len; j++)
			{
				entity* enemy = *((entity**) dynarray_get(entities, j));
				if (!enemy->dead && enemy->team != team)
				{
					float distance = map_distance_squared(enemy->pos, friendly->pos);
					
					if(distance < smallest_distance) smallest_distance = distance;

					anyone_alive = true;
				}
			}

			// if there's no enemies alive, don't calculate distance evaluation
			if(anyone_alive)
			{
				eval += per_distance_sq_weight * smallest_distance;
			}
		}
	}

	return eval;
}

float evaluate_cover(team team)
{
	float eval = 0.0f;

	float in_cover_no_enemy_in_direction_weight = 0.01f;
	float in_cover_enemy_in_direction_weight = 0.25f;

	// check if they are in cover from each angle (and prefer being in cover if there is an enemy in that direction)
	for (u32 i = 0; i < entities->len; i++)
	{
		entity* friendly = *((entity**) dynarray_get(entities, i));

		if (!friendly->dead && friendly->team == team)
		{
			for (s8 x = -1; x <= 1; x++)
			{
				for (s8 z = -1; z <= 1; z++)
				{
					bool enemy_in_direction = false;

					vec3 block_pos = friendly->pos;
					block_pos.x += x;
					block_pos.z += z;

					if (map_is_cover(block_pos))
					{
						vec3 cover_to_covered_vector = glm::normalize(block_pos - friendly->pos);

						for (u32 j = 0; j < entities->len; j++)
						{
							entity* enemy = *((entity**) dynarray_get(entities, j));

							if(!enemy->dead && enemy->team != team)
							{
								vec3 cover_to_enemy = glm::normalize(enemy->pos - block_pos);

								float angle = glm::dot(cover_to_covered_vector, cover_to_enemy);

								// if the angle is negative, that means that the cover is behind them (they're not covered!)
								if (angle >= 0.75f)
								{
									enemy_in_direction = true;
									break;
								}
							}
						}

						eval += enemy_in_direction ? in_cover_enemy_in_direction_weight : in_cover_no_enemy_in_direction_weight;
					}
				}
			}
		}
	}

	return eval;
}

evaluation board_evaluate(team team)
{
	evaluation eval = {0};

	eval.shot_chance = evaluate_shot_chance(team);
	eval.health = evaluate_health(team);
	eval.distance = evaluate_distance_to_enemy(team);
	eval.cover = evaluate_cover(team);

	eval.total = eval.shot_chance + eval.health + eval.distance + eval.cover;

	return eval;
}

void board_evaluation_print(evaluation eval)
{
	//printf("Board eval %f: shot chance: %f, health: %f, distance: %f, cover: %f\n", eval.total, eval.shot_chance, eval.health, eval.distance, eval.cover);
}