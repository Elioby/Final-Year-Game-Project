#include "action.h"

#include "entity.h"
#include "map.h"
#include "gui.h"
#include "input.h"
#include "actionbar.h"
#include "board_eval.h"

#define ACTION_SHOOT_DAMAGE 6

action_mode current_action_mode;

// nothing functions
void action_perform_nothing(entity* ent, vec3 target);
action_evaluation action_evaluate_nothing(entity* ent);

// move functions
action_evaluation action_evaluate_move(entity* ent);
void action_perform_move(entity* ent, vec3 target);

// shoot functions
void action_perform_shoot(entity* ent, vec3 target);
action_evaluation action_evaluate_shoot(entity* ent);

std::vector<action> actions;

action action_nothing;

void action_init()
{
	action_nothing = { 0 };
	action_nothing.name = "nothing";
	action_nothing.perform = action_perform_nothing;
	action_nothing.evaluate = action_evaluate_nothing;

	action move_action = { 0 };
	move_action.name = "move";
	move_action.perform = action_perform_move;
	move_action.evaluate = action_evaluate_move;
	actions.push_back(move_action);

	action shoot_action = { 0 };
	shoot_action.name = "shoot";
	shoot_action.perform = action_perform_shoot;
	shoot_action.evaluate = action_evaluate_shoot;
	actions.push_back(shoot_action);
}

void action_perform_nothing(entity* ent, vec3 target) { }

action_evaluation action_evaluate_nothing(entity* ent)
{
	action_evaluation eval = { 0 };

	eval.valid = true;
	eval.eval = evaluate_board(ent->team);

	return eval;
}

void action_perform_move(entity* ent, vec3 target)
{
	ent->pos = target;
}

action_evaluation action_evaluate_move(entity* ent)
{
	vec3 original_position = ent->pos;

	vec3 best_target = vec3();
	float best_move_eval = -FLT_MAX;

	for (u32 x = 0; x < map_max_x; x++)
	{
		for (u32 z = 0; z < map_max_z; z++)
		{
			vec3 move_target = vec3(x, 0, z);

			if (map_is_cover_at_block(move_target)) continue;

			if (map_get_entity_at_block(move_target) != NULL) continue;

			ent->pos = move_target;

			float move_eval = evaluate_board(ent->team);

			if (move_eval > best_move_eval)
			{
				best_target = move_target;
				best_move_eval = move_eval;
			}
		}
	}

	action_evaluation eval = { 0 };

	if (best_move_eval > 0)
	{
		eval.target = best_target;
		eval.eval = best_move_eval;
		eval.valid = true;
	}
	else
	{
		eval.eval = -FLT_MAX;
		eval.valid = false;
	}

	ent->pos = original_position;

	return eval;
}

void action_perform_shoot(entity* ent, entity* target_ent, bool temp)
{
	entity_health_change(target_ent, ent, -ACTION_SHOOT_DAMAGE, temp);
}

void action_perform_shoot(entity* ent, vec3 target)
{
	entity* target_ent = map_get_entity_at_block(target);

	action_perform_shoot(ent, target_ent, false);
}

action_evaluation action_evaluate_shoot(entity* ent)
{
	entity* highest_eval_ent = NULL;
	float highest_eval = -FLT_MAX;

	// find the best entity to shoot
	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* target_ent = entities[i];

		if (!entity_is_same_team(target_ent, ent))
		{
			if (!map_has_los(ent, target_ent)) continue;

			i32 original_hp = target_ent->health;

			action_perform_shoot(ent, target_ent, true);

			// @Todo: talk to frank about this. We have a chance to hit the shot, how should this effect the evaluation?
			float eval = evaluate_board(ent->team);

			// if the dmg done was > previous hp, just heal back the previous health (otherwise they gain health)
			i32 heal_amount = ACTION_SHOOT_DAMAGE;

			if (heal_amount > original_hp) heal_amount = original_hp;

			// heal them back up (it will also res them if they're dead, since this is temp mode)
			entity_health_change(target_ent, ent, heal_amount, true);

			if (eval > highest_eval)
			{
				highest_eval_ent = target_ent;
				highest_eval = eval;
			}
		}
	}

	action_evaluation eval = { 0 };

	if (highest_eval_ent)
	{
		eval.target = highest_eval_ent->pos;
		eval.valid = true;
		eval.eval = highest_eval;
	}
	else
	{
		eval.valid = false;
		eval.eval = -FLT_MAX;
	}

	return eval;
}

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
			}
			else if (current_action_mode == ACTION_MODE_MOVE)
			{
				// we can only move to free blocks
				if (!clicked_entity)
				{
					if (!map_is_cover_at_block(selected_block))
					{
						selected_entity->pos = selected_block;
					}
					else
					{
						actionbar_set_msg("Invalid move position", 2.0f);
					}
				}
				else
				{
					selected_entity = clicked_entity;
				}

				current_action_mode = ACTION_MODE_SELECT_UNITS;
			}
			else if (current_action_mode == ACTION_MODE_SHOOT)
			{
				if (clicked_entity && clicked_entity != selected_entity && !entity_is_same_team(selected_entity, clicked_entity))
				{
					if (selected_entity->ap > 30)
					{
						bool has_los = map_has_los(selected_entity, clicked_entity);

						if(has_los)
						{
							float los_amount = map_get_los_angle(selected_entity, clicked_entity);
							if(los_amount > 0.0f)
							{
								printf("shoot with chance %f\n", los_amount);
								// @Todo: renable! entity_health_change(clicked_entity, selected_entity, -6);
								// @Todo: renable! current_action_mode = ACTION_MODE_SELECT_UNITS;
								// @Todo: renable! selected_entity->ap -= 30;
							}
							else
							{
								actionbar_set_msg("No LOS", 2.0f);
							}
						}
						else
						{
							actionbar_set_msg("No LOS", 2.0f);
						}
					}
					else
					{
						actionbar_set_msg("Not enough AP", 2.0f);
					}
				}
				else
				{
					actionbar_set_msg("Invalid target", 2.0f);
				}
			}
			else if (current_action_mode == ACTION_MODE_THROW)
			{
				if (selected_entity->ap > 30)
				{
					// @Todo: maybe we should have some abstract sense of "objects" that are on the map so we can remove them all together?
					for (u32 i = 0; i < entities.size(); i++)
					{
						entity* ent = entities[i];

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

					selected_entity->ap -= 30;
				}
				else
				{
					// @Todo: abstract ap use out?
					actionbar_set_msg("Not enough AP", 2.0f);
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
		}
	}
}