#include "action.h"

#include "entity.h"
#include "map.h"
#include "gui.h"
#include "input.h"
#include "actionbar.h"

action_mode current_action_mode;

// @Todo: move
cover* map_get_adjacent_cover(vec3 start, vec3 closest_to)
{
	float smallest_distance = 0.0f;
	cover* closest_cover = NULL;

	for(i8 x = -1; x <= 1; x++)
	{
		for(i8 y = -1; y <= 1; y++)
		{
			vec3 block_pos = start;
			block_pos.x += x;
			block_pos.y += y;

			cover* cov = map_get_cover_at_block(block_pos);
			if(cov)
			{
				float distance = map_distance_squared(start, cov->pos);
				if(!closest_cover || smallest_distance < distance)
				{
					closest_cover = cov;
					smallest_distance = distance;
				}
			}
		}
	}

	return closest_cover;
}

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
						// how leinient is the los algorithm, in blocks
						float lean_distance = 0.5f;
						// maybe we only need to check along the opposite axis of the direction
						if (map_check_los(selected_entity->pos, clicked_entity->pos) ||
							map_check_los(vec3(selected_entity->pos.x + lean_distance, selected_entity->pos.y, selected_entity->pos.z), clicked_entity->pos) ||
							map_check_los(vec3(selected_entity->pos.x - lean_distance, selected_entity->pos.y, selected_entity->pos.z), clicked_entity->pos) ||
							map_check_los(vec3(selected_entity->pos.x, selected_entity->pos.y, selected_entity->pos.z + lean_distance), clicked_entity->pos) ||
							map_check_los(vec3(selected_entity->pos.x, selected_entity->pos.y, selected_entity->pos.z - lean_distance), clicked_entity->pos) ||
							map_check_los(clicked_entity->pos, clicked_entity->pos) ||
							map_check_los(vec3(clicked_entity->pos.x + lean_distance, clicked_entity->pos.y, clicked_entity->pos.z), selected_entity->pos) ||
							map_check_los(vec3(clicked_entity->pos.x - lean_distance, clicked_entity->pos.y, clicked_entity->pos.z), selected_entity->pos) ||
							map_check_los(vec3(clicked_entity->pos.x, clicked_entity->pos.y, clicked_entity->pos.z + lean_distance), selected_entity->pos) ||
							map_check_los(vec3(clicked_entity->pos.x, clicked_entity->pos.y, clicked_entity->pos.z - lean_distance), selected_entity->pos))
						{
							float los_amount = 1.0f;

							cover* closest_cover = map_get_adjacent_cover(clicked_entity->pos, selected_entity->pos);

							if(closest_cover)
							{
								vec3 cover_to_covered_vector = glm::normalize(closest_cover->pos - clicked_entity->pos);
								vec3 cover_to_shooter = glm::normalize(selected_entity->pos - closest_cover->pos);

								float angle = glm::dot(cover_to_covered_vector, cover_to_shooter);

								// if the angle is negative, that means that the cover is behind them (they're not covered!)
								if(angle >= 0.0f)
								{
									los_amount = (1.0f - angle);
								}

								printf("cover: %f, %f, %f\n", closest_cover->pos.x, closest_cover->pos.y, closest_cover->pos.z);
							}

							printf("los amount: %f\n", los_amount);

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
					for (u32 i = 0; i < cover_list.size(); i++)
					{
						cover* cov = cover_list[i];

						// euclidean distance
						float distance_squared = map_distance_squared(selected_block, cov->pos);

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

			map_debug_los.clear();
		}
	}
}