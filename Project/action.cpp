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

						// euclidean distance
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

			//map_add_cover(input_mouse_block_pos);
			printf("%f, %f\n", input_mouse_block_pos.x, input_mouse_block_pos.z);
		}
	}
}