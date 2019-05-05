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
#include "window.h"

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
void get_targets_nothing(entity* ent, dynarray* targets, bool monte_carlo) {};

// move functions
action_undo_data* gather_undo_data_move(entity* ent, vec3 target);
void perform_move(entity* ent, vec3 target, bool temp);
void undo_move(entity* ent, action_undo_data* undo_data);
void get_targets_move(entity* ent, dynarray* targets, bool monte_carlo);

// shoot functions
action_undo_data* gather_undo_data_shoot(entity* ent, vec3 target);
void perform_shoot(entity* ent, vec3 target, bool temp);
void undo_shoot(entity* ent, action_undo_data* undo_data);
void get_targets_shoot(entity* ent, dynarray* targets, bool monte_carlo);

// actions not including "nothing" action
action actions[2];

action action_nothing;
action action_move;
action action_shoot;

dynarray* action_move_targets = dynarray_create((ACTION_MOVE_RADIUS + 1) * ACTION_MOVE_RADIUS * 2, sizeof(vec3));

void action_init()
{
	action_nothing = { 0 };
	action_nothing.name = "nothing";
	action_nothing.perform = perform_nothing;
	action_nothing.gather_undo_data = gather_undo_data_nothing;
	action_nothing.undo = undo_nothing;
	action_nothing.get_targets = get_targets_nothing;

	action_move = { 0 };
	action_move.name = "move";
	action_move.perform = perform_move;
	action_move.gather_undo_data = gather_undo_data_move;
	action_move.undo = undo_move;
	action_move.get_targets = get_targets_move;
	actions[0] = action_move;

	action_shoot = { 0 };
	action_shoot.name = "shoot";
	action_shoot.perform = perform_shoot;
	action_shoot.gather_undo_data = gather_undo_data_shoot;
	action_shoot.undo = undo_shoot;
	action_shoot.get_targets = get_targets_shoot;
	actions[1] = action_shoot;
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
	entity_move(ent, target);
}

void undo_move(entity* ent, action_undo_data* undo_data)
{
	action_undo_data_move* undo_data_move = (action_undo_data_move*) undo_data;
	entity_move(ent, undo_data_move->old_pos);
}

struct point
{
	u32 x, z;

	float distance_to_start;
};

bool get_next_target_move_direction(vec3 pos, point last, dynqueue* moveable_positions_queue, bool* already_searched)
{
	if (pos.x >= 0 && pos.z >= 0 && pos.x < map_max_x && pos.z < map_max_z)
	{
		bool* already_searched_this = already_searched + (u32) pos.x + (u32)pos.z * map_max_x;
		
		if (!(*already_searched_this) && map_is_movable(pos))
		{
			*already_searched_this = true;

			point next = { 0 };
			next.x = (u32) pos.x;
			next.z = (u32) pos.z;
			next.distance_to_start = last.distance_to_start + 1;

			if (next.distance_to_start < ACTION_MOVE_RADIUS)
			{
				dynqueue_push(moveable_positions_queue, &next);
			}

			return true;
		}
	}

	return false;
}

bool* already_searched = (bool*) debug_calloc(map_max_x * map_max_z, sizeof(bool));

void try_add_target(dynarray* targets, vec3 pos)
{
	double random = (double) rand() / RAND_MAX;

	// add 70% of in cover spots and 10% of out of cover spots
	if(random < 0.1 || (random < 0.7 && map_is_adjacent_to_cover(pos)))
	{
		dynarray_add(targets, &pos);
	}
}

void get_targets_move(entity* ent, dynarray* targets, bool monte_carlo)
{
	dynqueue* moveable_positions_queue = dynqueue_create(ACTION_MOVE_RADIUS * 4, sizeof(point));
	point start = { 0 };
	start.x = (u32) ent->pos.x;
	start.z = (u32) ent->pos.z;
	start.distance_to_start = 0;

	dynqueue_push(moveable_positions_queue, &start);

	memset(already_searched, 0, map_max_x * map_max_z * sizeof(bool));

	while (moveable_positions_queue->len > 0)
	{
		point p = *(point*) dynqueue_front(moveable_positions_queue);

		vec3 pos = vec3(p.x + 1, 0, p.z);
		if(get_next_target_move_direction(pos, p, moveable_positions_queue, already_searched))
		{
			if(monte_carlo) try_add_target(targets, pos);
			else dynarray_add(targets, &pos);
		}

		pos = vec3(p.x - 1, 0, p.z);
		if (get_next_target_move_direction(pos, p, moveable_positions_queue, already_searched))
		{
			if (monte_carlo) try_add_target(targets, pos);
			else dynarray_add(targets, &pos);
		}

		pos = vec3(p.x, 0, p.z + 1);
		if (get_next_target_move_direction(pos, p, moveable_positions_queue, already_searched))
		{
			if (monte_carlo) try_add_target(targets, pos);
			else dynarray_add(targets, &pos);
		}

		pos = vec3(p.x, 0, p.z - 1);
		if (get_next_target_move_direction(pos, p, moveable_positions_queue, already_searched))
		{
			if (monte_carlo) try_add_target(targets, pos);
			else dynarray_add(targets, &pos);
		}

		dynqueue_pop(moveable_positions_queue);
	}

	dynqueue_destroy(moveable_positions_queue);
}

action_undo_data* gather_undo_data_shoot(entity* ent, vec3 target)
{
	// @Speed: replace this with a custom stack push?
	action_undo_data_shoot* undo_data = (action_undo_data_shoot*) debug_malloc(sizeof(action_undo_data_shoot));

	entity* target_ent = map_get_entity_at_block(target);

	debug_assert(target_ent, "Tried to gather undo data while shooting a target entity that isn't there");

	undo_data->damage_taken = min(target_ent->health, ACTION_SHOOT_DAMAGE);
	undo_data->target_ent = target_ent;

	return undo_data;
}

void perform_shoot(entity* ent, vec3 target, bool temp)
{
	entity* target_ent = map_get_entity_at_block(target);

	if (temp)
	{
		entity_health_change(target_ent, ent, -ACTION_SHOOT_DAMAGE, temp);
	}
	else
	{
		float los_amount = map_get_los_angle(ent, target_ent);
		if (los_amount > 0.0f)
		{

			double random = (double) rand() / (double) RAND_MAX;

			if (random <= los_amount)
			{
				entity_health_change(target_ent, ent, -ACTION_SHOOT_DAMAGE, temp);
			}
			else
			{
				actionbar_combatlog_add("Entity %i missed a shot on entity %i", ent->id, target_ent->id);
			}
		}
	}
}

void undo_shoot(entity* ent, action_undo_data* undo_data)
{
	action_undo_data_shoot* undo_data_shoot = (action_undo_data_shoot*) undo_data;

	// heal them back up, if they were dead this ressurects them
	entity_health_change(undo_data_shoot->target_ent, ent, undo_data_shoot->damage_taken, true);
}

void get_targets_shoot(entity* ent, dynarray* targets, bool monte_carlo)
{
	for (u32 i = 0; i < entities->len; i++)
	{
		entity* target_ent = *((entity**) dynarray_get(entities, i));
		
		if (!target_ent->dead && !entity_is_same_team(target_ent, ent))
		{
			if (!map_has_los(ent, target_ent)) continue;
		
			dynarray_add(targets, &target_ent->pos);
		}
	}
}

bool was_just_pressing_tab = false;

void action_update()
{
	// tab switches between units
	if(was_just_pressing_tab && glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
	{
		bool changed_entity = false;

		for(u32 i = 0; i < entities->len; i++)
		{
			entity* ent = *(entity**) dynarray_get(entities, i);

			if(ent->team == TEAM_FRIENDLY && (!selected_entity || ent == selected_entity))
			{
				if(!selected_entity)
				{
					selected_entity = ent;
					changed_entity = true;
				}
				else
				{
					for (u32 j = i + 1; true; j++)
					{
						if (j + 1 >= entities->len) j = 0;

						entity* next_ent = *(entity**) dynarray_get(entities, j);

						if(next_ent == selected_entity)
						{
							// there are no other entities
							break;
						}
						else if (next_ent->team == TEAM_FRIENDLY && next_ent->ap > 0)
						{
							selected_entity = next_ent;
							changed_entity = true;
							break;
						}
					}
				}

				break;
			}
		}

		if(changed_entity) action_switch_mode(ACTION_MODE_SELECT_UNITS);
	}

	was_just_pressing_tab = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;

	// activate different modes with keyboard buttons
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		if (action_get_action_mode() != ACTION_MODE_SELECT_UNITS)
		{
			action_switch_mode(ACTION_MODE_SELECT_UNITS);
		}
	}

	if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
	{
		if (selected_entity && action_get_action_mode() != ACTION_MODE_MOVE)
		{
			action_move_mode(0);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
	{
		if (selected_entity && action_get_action_mode() != ACTION_MODE_SHOOT)
		{
			action_switch_mode(ACTION_MODE_SELECT_UNITS);
			action_shoot_mode(0);
		}
	}

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
	{
		if (selected_entity)
		{
			action_do_nothing(0);
		}
	}

	// action usage
	if (!gui_handled_click())
	{
		if (input_mouse_button_left == INPUT_MOUSE_BUTTON_UP_START)
		{
			vec3 selected_block = input_mouse_block_pos;

			entity* clicked_entity = map_get_entity_at_block(selected_block);

			// @Todo: action cleanup? 
			if (action_get_action_mode() == ACTION_MODE_SELECT_UNITS)
			{
				if(clicked_entity && clicked_entity->team == TEAM_FRIENDLY)
				{
					selected_entity = clicked_entity;
				}
			}
			else if (action_get_action_mode() == ACTION_MODE_MOVE)
			{
				bool valid_move_position = false;

				for(u32 i = 0; i < action_move_targets->len; i++)
				{
					vec3 pos = *(vec3*) dynarray_get(action_move_targets, i);

					if (map_pos_equal(selected_block, pos))
					{
						valid_move_position = true;
						break;
					}
				}

				if (valid_move_position || TESTING_MODE)
				{
					action_move.perform(selected_entity, selected_block, false);
					selected_entity->ap -= 1;

					dynarray_clear(action_move_targets);

					if (selected_entity->ap > 0)
					{
						action_move_mode(0);
					}
					else
					{
						action_switch_mode(ACTION_MODE_SELECT_UNITS);
					}
				}
				else
				{
					actionbar_set_msg("Invalid move position", 3.0f);
				}
			}
		}
		else if (input_mouse_button_right == INPUT_MOUSE_BUTTON_UP_START)
		{
			action_switch_mode(ACTION_MODE_SELECT_UNITS);
			selected_entity = NULL;
			dynarray_clear(action_move_targets);

			/*dynarray_clear(map_road_segments);
			map_gen();*/
		}
	}
}

// we don't want anyhting above accessing this variable
action_mode current_action_mode;

action_mode action_get_action_mode()
{
	return current_action_mode;
}

void action_switch_mode(action_mode mode)
{
	actionbar_switch_off_mode(current_action_mode);

	current_action_mode = mode;
}