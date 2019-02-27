#include "action.h"

#include "entity.h"
#include "map.h"
#include "gui.h"
#include "input.h"
#include "actionbar.h"

action_mode current_action_mode;

void action_update()
{
	if (!gui_handled_click())
	{
		if (input_mouse_button_left == INPUT_MOUSE_BUTTON_UP_START)
		{
			vec3 selected_block = input_mouse_block_pos;

			entity* clicked_entity = entity_get_at_block(selected_block);

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
					if (map_get_cover_at_block(selected_block) == NULL)
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
						// maybe we only need to check along the opposite axis of the direction
						if (map_check_los(selected_entity->pos, clicked_entity->pos) ||
							map_check_los(vec3(selected_entity->pos.x + 1, selected_entity->pos.y, selected_entity->pos.z), clicked_entity->pos) ||
							map_check_los(vec3(selected_entity->pos.x - 1, selected_entity->pos.y, selected_entity->pos.z), clicked_entity->pos) ||
							map_check_los(vec3(selected_entity->pos.x, selected_entity->pos.y, selected_entity->pos.z + 1), clicked_entity->pos) ||
							map_check_los(vec3(selected_entity->pos.x, selected_entity->pos.y, selected_entity->pos.z - 1), clicked_entity->pos))
						{
							entity_health_change(clicked_entity, selected_entity, -6);
							current_action_mode = ACTION_MODE_SELECT_UNITS;
							selected_entity->ap -= 30;
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

						// euclidean distance
						float distance_squared = pow(abs(selected_block.x - ent->pos.x), 2) + pow(abs(selected_block.z - ent->pos.z), 2);

						if (distance_squared < 12)
						{
							entity_health_change(ent, selected_entity, -6);
						}
					}

					// @Cleanup: dupe code
					for (u32 i = 0; i < cover_list.size(); i++)
					{
						cover* cov = cover_list[i];

						// euclidean distance
						float distance_squared = pow(abs(selected_block.x - cov->pos.x), 2) + pow(abs(selected_block.z - cov->pos.z), 2);

						if (distance_squared < 12)
						{
							cover_list.erase(cover_list.begin() + i);

							// since we removed one from the list, go back one index
							i--;
						}
					}

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