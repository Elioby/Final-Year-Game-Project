#include <stdio.h>

#include <glm/glm.hpp>

#include <bgfx/c99/bgfx.h>

#include "general.h"
#include "window.h"
#include "graphics.h"
#include "input.h"
#include "gui.h"
#include "assets.h"
#include "map.h"
#include "entity.h"
#include "font.h"
#include "dynstr.h"
#include "actionbar.h"
#include "action.h"

void draw();
void update(float dt);
void camera_update(float dt);

int main()
{
	window_init();
	graphics_init(WINDOW_WIDTH, WINDOW_HEIGHT);
	input_init();
	gui_init();
	assets_init();
	map_init();
	actionbar_init();

	float lastTime = 0;
	float time, dt;

	while(!glfwWindowShouldClose(window))
	{
		
		time = (float) glfwGetTime();
		dt = time - lastTime;
		lastTime = time;

		glfwPollEvents();
		update(dt);

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx_touch(0);

		bgfx_dbg_text_clear(0, false);

		bgfx_dbg_text_printf(0, 0, 0x0f, "Last frame time: %.2fms, FPS: %.0f", dt * 1000.0f, 1 / dt);

		draw();

		bgfx_frame(false);

		input_end_frame();
	}

	bgfx_shutdown();
	glfwTerminate();

	return 0;
}

void update(float dt)
{
	camera_update(dt);
	input_update();
	actionbar_update(dt);

	// @Todo: move to "entity_update"
	// if our unit dies on our turn, unselect them!
	if(selected_entity != NULL && selected_entity->dead) selected_entity = NULL;

	// @Todo: should we remove from the list when they die?
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* ent = entities[i];

		if(ent->dead) entities.erase(entities.begin() + i);
	}

	// end entity update

	bool click_handled_by_gui = gui_update();

	if(!click_handled_by_gui)
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
					if(map_get_cover_at_block(selected_block) == NULL)
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
			else if(current_action_mode == ACTION_MODE_SHOOT)
			{
			    if(clicked_entity && clicked_entity != selected_entity && !entity_is_same_team(selected_entity, clicked_entity))
				{
					if(selected_entity->ap > 30)
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
				if(selected_entity->ap > 30)
				{
					// @Todo: maybe we should have some abstract sense of "objects" that are on the map so we can remove them all together?
					for(u32 i = 0; i < entities.size(); i++)
					{
						entity* ent = entities[i];

						// euclidean distance
						float distance_squared = pow(abs(selected_block.x - ent->pos.x), 2) + pow(abs(selected_block.z - ent->pos.z), 2);

						if(distance_squared < 12)
						{
							entity_health_change(ent, selected_entity, -6);
						}
					}

					// @Cleanup: dupe code
					for(u32 i = 0; i < cover_list.size(); i++)
					{
						cover* cov = cover_list[i];

						// euclidean distance
						float distance_squared = pow(abs(selected_block.x - cov->pos.x), 2) + pow(abs(selected_block.z - cov->pos.z), 2);

						if(distance_squared < 12)
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

void draw()
{
	// draw selected tile
	graphics_draw_mesh(cube_mesh, graphics_create_model_matrix(input_mouse_block_pos, 0.0f, vec3(1.0f), vec3(1.0f, 0.1f, 1.0f)));

	// draw selected entity
	if(selected_entity)
	{
		//graphics_draw_image(selected_entity_image, graphics_create_model_matrix(vec3(3.0f, 0.01f, 3.0f), radians(0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.5f)));
	}

	// @Todo: move to map_draw
	// draw cover
	for(u32 i = 0; i < cover_list.size(); i++)
	{
		cover* cov = cover_list[i];
		graphics_draw_mesh(cube_mesh, graphics_create_model_matrix(cov->pos, 0.0f, vec3(1.0f), vec3(1.0f, 2.0f, 1.0f)));
	}

	// @Todo: move to map_draw
	// draw entites and healthbars
	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* ent = entities[i];

		if(!ent->dead)
		{
			vec3 healthbox_aspect = vec3(1.0f, 1.0f / 3.0f, 1.0f);

			if (ent->health > 0) 
			{
				image img = fhealthbar_image;
				if(ent->enemy) img = ehealthbar_image;
				graphics_draw_image(img, graphics_create_model_matrix(vec3(ent->pos.x + 0.033333f, ent->pos.y + 2.038f, ent->pos.z + 0.5f), 0.0f, vec3(1.0f),
					vec3((0.5f - 0.1f / 3.0f) * (ent->health / (float)ent->max_health), 0.1285f, 1.0f)));
			}

			graphics_draw_image(healthbox_image, graphics_create_model_matrix(vec3(ent->pos.x, ent->pos.y + 2.0f, ent->pos.z + 0.5f), 0.0f, vec3(1.0f), healthbox_aspect * 0.5f));

			graphics_draw_mesh(ent->mesh, graphics_create_model_matrix(ent->pos, 0.0f, vec3(1.0f), vec3(1.0f)));
		}
	}

	// @Todo: move to map_draw
	// draw terrain
	graphics_draw_mesh(terrain_mesh, graphics_create_model_matrix(vec3(0.0f), 0.0f, vec3(1.0f), vec3(1.0f)));

	// draw action bar
	actionbar_draw();
}

