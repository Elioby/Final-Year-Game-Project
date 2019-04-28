#include "action.h"

#include "libmorton\morton.h"

#include "entity.h"
#include "map.h"
#include "map_gen.h"
#include "gui.h"
#include "input.h"
#include "actionbar.h"
#include "board_eval.h"
#include "dynqueue.h"

#include <queue>

#define ACTION_SHOOT_DAMAGE 6
#define ACTION_MOVE_RADIUS 8
#define ACTION_GRENADE_RADIUS 3
#define ACTION_GRENADE_DAMAGE 4

action_mode current_action_mode;

struct action_undo_data_move : action_undo_data
{
	vec3 old_pos;
};

struct action_undo_data_shoot : action_undo_data
{
	u32 damage_taken;
	entity* target_ent;
};

// nothing functions
action_undo_data* gather_undo_data_nothing(entity* ent, vec3 target) { return NULL; }
void perform_nothing(entity* ent, vec3 target, bool temp) {}
void undo_nothing(entity* ent, action_undo_data* undo_data) {}
bool get_next_target_nothing(entity* ent, u32* last_index, vec3* result);

// move functions
action_undo_data* gather_undo_data_move(entity* ent, vec3 target);
void perform_move(entity* ent, vec3 target, bool temp);
void undo_move(entity* ent, action_undo_data* undo_data);
bool get_next_target_move(entity* ent, u32* last_index, vec3* result);

// shoot functions
action_undo_data* gather_undo_data_shoot(entity* ent, vec3 target);
void perform_shoot(entity* ent, vec3 target, bool temp);
void undo_shoot(entity* ent, action_undo_data* undo_data);
bool get_next_target_shoot(entity* ent, u32* last_index, vec3* result);

// actions not including "nothing" action
action actions[2];

action action_nothing;

void action_init()
{
	action_nothing = { 0 };
	action_nothing.name = "nothing";
	action_nothing.perform = perform_nothing;
	action_nothing.gather_undo_data = gather_undo_data_nothing;
	action_nothing.undo = undo_nothing;
	action_nothing.get_next_target = get_next_target_nothing;

	action move_action = { 0 };
	move_action.name = "move";
	move_action.perform = perform_move;
	move_action.gather_undo_data = gather_undo_data_move;
	move_action.undo = undo_move;
	move_action.get_next_target = get_next_target_move;
	actions[0] = move_action;

	action shoot_action = { 0 };
	shoot_action.name = "shoot";
	shoot_action.perform = perform_shoot;
	shoot_action.gather_undo_data = gather_undo_data_shoot;
	shoot_action.undo = undo_shoot;
	shoot_action.get_next_target = get_next_target_shoot;
	actions[1] = shoot_action;
}

bool get_next_target_nothing(entity* ent, u32* last_index, vec3* result)
{
	// we only return one valid position for doing nothing
	if(*last_index != 0) return false;

	*last_index = 1;
	*result = ent->pos;
	return true;
}

action_undo_data* gather_undo_data_move(entity* ent, vec3 target)
{
	// @Speed: replace this with a custom stack push?
	action_undo_data_move* undo_data = (action_undo_data_move*) debug_malloc(sizeof(action_undo_data_move));

	undo_data->old_pos = ent->pos;

	return undo_data;
}

void perform_move(entity* ent, vec3 target, bool temp)
{
	ent->pos = target;
}

void undo_move(entity* ent, action_undo_data* undo_data)
{
	action_undo_data_move* undo_data_move = (action_undo_data_move*) undo_data;
	ent->pos = undo_data_move->old_pos;
}

struct point
{
	u32 x, z;

	float distance_to_start;
};

bool get_next_target_move_direction(bool* already_searched, vec3 pos, point last, dynqueue* moveable_positions_queue, u32* current_index, u32* last_index, vec3* result)
{
	if (pos.x >= 0 && pos.z >= 0 && pos.x < map_max_x && pos.z < map_max_z)
	{
		bool* already_searched_this = already_searched + (u32) pos.x + (u32) pos.z * map_max_x;

		if (!(*already_searched_this) && !map_is_cover_at_block(pos) && map_get_entity_at_block(pos) == NULL)
		{
			point next = { 0 };
			next.x = (u32) pos.x;
			next.z = (u32) pos.z;
			next.distance_to_start = last.distance_to_start + 1;

			*already_searched_this = true;

			if (next.distance_to_start < ACTION_MOVE_RADIUS)
			{
				dynqueue_push(moveable_positions_queue, &next);
			}

			*current_index = *current_index + 1;

			if (*current_index > *last_index)
			{
				*last_index = *current_index;
				*result = pos;
				return true;
			}
		}
	}

	return false;
}

// @Speed: this is very slow for no reason other than you call it everytime you want a new move position, so you have to recurse down more every time..
//          we could just return a dynarray of moves instead, it would be much quicker (although we would need a memory allocation)
bool get_next_target_move(entity* ent, u32* last_index, vec3* result)
{
	u32 i = 0;

	dynqueue* moveable_positions_queue = dynqueue_create(ACTION_MOVE_RADIUS * 4, sizeof(point));
	point start = { 0 };
	start.x = (u32) ent->pos.x;
	start.z = (u32) ent->pos.z;
	start.distance_to_start = 0;

	dynqueue_push(moveable_positions_queue, &start);

	// @Speed we do a calloc everytime we want a new target position..
	bool* already_searched = (bool*) debug_calloc(map_max_x * map_max_z, sizeof(bool));

	bool found_next = false;

	while (moveable_positions_queue->len > 0)
	{
		point p = *(point*) dynqueue_front(moveable_positions_queue);

		if (get_next_target_move_direction(already_searched, vec3(p.x + 1, 0, p.z), p, moveable_positions_queue, &i, last_index, result) ||
			get_next_target_move_direction(already_searched, vec3(p.x - 1, 0, p.z), p, moveable_positions_queue, &i, last_index, result) ||
			get_next_target_move_direction(already_searched, vec3(p.x, 0, p.z + 1), p, moveable_positions_queue, &i, last_index, result) ||
			get_next_target_move_direction(already_searched, vec3(p.x, 0, p.z - 1), p, moveable_positions_queue, &i, last_index, result))
		{
			found_next = true;
			break;
		}

		dynqueue_pop(moveable_positions_queue);
	}

	dynqueue_destroy(moveable_positions_queue);
	free(already_searched);
	return found_next;
}

action_undo_data* gather_undo_data_shoot(entity* ent, vec3 target)
{
	// @Speed: replace this with a custom stack push?
	action_undo_data_shoot* undo_data = (action_undo_data_shoot*) debug_malloc(sizeof(action_undo_data_shoot));

	entity* target_ent = map_get_entity_at_block(target);

	debug_assert(target_ent, "Tried to gather undo data while shooting a target entity that isn't there");

	undo_data->damage_taken = glm::min(target_ent->health, ACTION_SHOOT_DAMAGE);
	undo_data->target_ent = target_ent;

	return undo_data;
}

void perform_shoot(entity* ent, vec3 target, bool temp)
{
	entity* target_ent = map_get_entity_at_block(target);

	entity_health_change(target_ent, ent, -ACTION_SHOOT_DAMAGE, temp);
}

void undo_shoot(entity* ent, action_undo_data* undo_data)
{
	action_undo_data_shoot* undo_data_shoot = (action_undo_data_shoot*) undo_data;

	// heal them back up, if they were dead this ressurects them
	entity_health_change(undo_data_shoot->target_ent, ent, undo_data_shoot->damage_taken, true);
}

bool get_next_target_shoot(entity* ent, u32* last_index, vec3* result)
{
	for (u32 i = *last_index; i < entities->len; i++)
	{
		entity* target_ent = (entity*) dynarray_get(entities, i);
		
		if (!target_ent->dead && !entity_is_same_team(target_ent, ent))
		{
			if (!map_has_los(ent, target_ent)) continue;
		
			*last_index = i + 1;
			*result = target_ent->pos;
			return true;
		}
	}

	return false;
}

//action_evaluation action_evaluate_move(entity* ent)
//{
//	vec3 original_position = ent->pos;
//
//	vec3 best_target = vec3();
//	float best_move_eval = -FLT_MAX;
//
//	for (u32 x = 0; x < map_max_x; x++)
//	{
//		for (u32 z = 0; z < map_max_z; z++)
//		{
//			vec3 move_target = vec3(x, 0, z);
//
//			if (map_is_cover_at_block(move_target)) continue;
//
//			if (map_get_entity_at_block(move_target) != NULL) continue;
//
//			ent->pos = move_target;
//
//			float move_eval = evaluate_board(ent->team);
//
//			if (move_eval > best_move_eval)
//			{
//				best_target = move_target;
//				best_move_eval = move_eval;
//			}
//		}
//	}
//
//	action_evaluation eval = { 0 };
//
//	if (best_move_eval > 0)
//	{
//		eval.target = best_target;
//		eval.eval = best_move_eval;
//		eval.valid = true;
//	}
//	else
//	{
//		eval.eval = -FLT_MAX;
//		eval.valid = false;
//	}
//
//	ent->pos = original_position;
//
//	return eval;
//}
//
//action_evaluation action_evaluate_shoot(entity* ent)
//{
//	entity* highest_eval_ent = NULL;
//	float highest_eval = -FLT_MAX;
//
//	// find the best entity to shoot
//	for (u32 i = 0; i < entities->len; i++)
//	{
//		entity* target_ent = (entity*) dynarray_get(entities, i);
//
//		if (!target_ent->dead && !entity_is_same_team(target_ent, ent))
//		{
//			if (!map_has_los(ent, target_ent)) continue;
//
//			i32 original_hp = target_ent->health;
//
//			action_perform_shoot(ent, target_ent, true);
//
//			float eval = evaluate_board(ent->team);
//
//			// if the dmg done was > previous hp, just heal back the previous health (otherwise they gain health)
//			i32 heal_amount = ACTION_SHOOT_DAMAGE;
//
//			if (heal_amount > original_hp) heal_amount = original_hp;
//
//			// heal them back up (it will also res them if they're dead, since this is temp mode)
//			entity_health_change(target_ent, ent, heal_amount, true);
//
//			if (eval > highest_eval)
//			{
//				highest_eval_ent = target_ent;
//				highest_eval = eval;
//			}
//		}
//	}
//
//	action_evaluation eval = { 0 };
//
//	if (highest_eval_ent)
//	{
//		eval.target = highest_eval_ent->pos;
//		eval.valid = true;
//		eval.eval = highest_eval;
//	}
//	else
//	{
//		eval.valid = false;
//		eval.eval = -FLT_MAX;
//	}
//
//	return eval;
//}

// @Todo: use the above code in the below code
void action_update()
{
	if (!gui_handled_click())
	{
		if (input_mouse_button_left == INPUT_MOUSE_BUTTON_UP_START)
		{
			vec3 selected_block = input_mouse_block_pos;

			entity* clicked_entity = map_get_entity_at_block(selected_block);

			// @Todo: action cleanup? 
			if (current_action_mode == ACTION_MODE_SELECT_UNITS)
			{
				selected_entity = clicked_entity;

				if(clicked_entity == NULL)
				{
					map_add_cover(input_mouse_block_pos);
				}
			}
			else if (current_action_mode == ACTION_MODE_MOVE)
			{
				// we can only move to free blocks
				if (!clicked_entity)
				{
					if (true)
					{
						if (!map_is_cover_at_block(selected_block))
						{
							selected_entity->pos = selected_block;
							selected_entity->ap -= 1;
						}
						else
						{
							actionbar_set_msg("Invalid move position", 3.0f);
						}
					}
					else
					{
						actionbar_set_msg("Not enough AP", 3.0f);
					}
				}
				else
				{
					selected_entity = clicked_entity;
					poses.clear();
				}

				current_action_mode = ACTION_MODE_SELECT_UNITS;
			}
			else if (current_action_mode == ACTION_MODE_SHOOT)
			{
				if (clicked_entity && clicked_entity != selected_entity && !entity_is_same_team(selected_entity, clicked_entity))
				{
					if (selected_entity->ap >= 1)
					{
						bool has_los = map_has_los(selected_entity, clicked_entity);

						if(has_los)
						{
							float los_amount = map_get_los_angle(selected_entity, clicked_entity);
							if(los_amount > 0.0f)
							{
								printf("shoot with chance %f\n", los_amount);

								double random = (double) rand() / (double) RAND_MAX;

								if(random <= los_amount)
								{
									entity_health_change(clicked_entity, selected_entity, -6);
									current_action_mode = ACTION_MODE_SELECT_UNITS;
								}
								else
								{
									actionbar_set_msg("Missed..", 3.0f);
								}

								selected_entity->ap -= 50;
							}
							else
							{
								actionbar_set_msg("No LOS", 3.0f);
							}
						}
						else
						{
							actionbar_set_msg("No LOS", 3.0f);
						}
					}
					else
					{
						actionbar_set_msg("Not enough AP", 3.0f);
					}
				}
				else
				{
					actionbar_set_msg("Invalid target", 3.0f);
				}
			}
			else if (current_action_mode == ACTION_MODE_THROW)
			{
				if (selected_entity->ap >= 50)
				{
					// @Todo: maybe we should have some abstract sense of "objects" that are on the map so we can remove them all together?
					for (u32 i = 0; i < entities->len; i++)
					{
						entity* ent = (entity*) dynarray_get(entities, i);

						// euclidean distance squared
						float distance_squared = map_distance_squared(selected_block, ent->pos);

						if (distance_squared < 12)
						{
							entity_health_change(ent, selected_entity, -6);
						}
					}

					// @Cleanup: dupe code
					// @Todo: fix nade code!
					//for (u32 i = 0; i < cover_list.size(); i++)
					//{
					//	cover* cov = cover_list[i];

					//	// euclidean distance
					//	float distance_squared = map_distance_squared(selected_block, cov->pos);

					//	if (distance_squared < 12)
					//	{
					//		cover_list.erase(cover_list.begin() + i);

					//		// since we removed one from the list, go back one index
					//		i--;
					//	}
					//}

					selected_entity->ap -= 50;
				}
				else
				{
					// @Todo: abstract ap use out?
					actionbar_set_msg("Not enough AP", 3.0f);
				}

				current_action_mode = ACTION_MODE_SELECT_UNITS;
			}
			else
			{
				printf("Action mode unknown!\n");
			}
		}
		else if (input_mouse_button_right == INPUT_MOUSE_BUTTON_UP_START)
		{
			current_action_mode = ACTION_MODE_SELECT_UNITS;
			selected_entity = NULL;
			poses.clear();

			/*dynarray_clear(map_road_segments);
			map_gen();*/
		}
	}
}