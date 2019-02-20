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

void gui_setup();
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
	
	// @Todo: temporary?
	gui_setup();

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

// @Todo: move
std::vector<button*> action_bar_buttons;

typedef enum action_mode {
	ACTION_MODE_SELECT_UNITS,
	ACTION_MODE_MOVE,
	ACTION_MODE_SHOOT,
	ACTION_MODE_THROW
} action_mode_t;

action_mode current_action_mode = ACTION_MODE_SELECT_UNITS;

void action_move_mode()
{
	current_action_mode = ACTION_MODE_MOVE;
}

void action_shoot_mode()
{
	current_action_mode = ACTION_MODE_SHOOT;
}

void action_throw_mode()
{
	current_action_mode = ACTION_MODE_THROW;
}

void gui_setup()
{
	button* move_button = gui_create_button();
	move_button->width = graphics_projection_width / 20;
	move_button->height = move_button->width;
	move_button->x = graphics_projection_width / 2 - (move_button->width * 4) / 2;
	move_button->y = move_button->height / 8;
	move_button->icon_img = action_move_image;
	move_button->bg_img = action_image;
	move_button->hover_bg_img = action_hover_image;
	move_button->click_callback = action_move_mode;
	action_bar_buttons.push_back(move_button);

	button* shoot_button = gui_create_button();
	shoot_button->width = graphics_projection_width / 20;
	shoot_button->height = shoot_button->width;
	shoot_button->x = graphics_projection_width / 2 - shoot_button->width / 2;
	shoot_button->y = shoot_button->height / 8;
	shoot_button->icon_img = action_shoot_image;
	shoot_button->bg_img = action_image;
	shoot_button->hover_bg_img = action_hover_image;
	shoot_button->click_callback = action_shoot_mode;
	action_bar_buttons.push_back(shoot_button);

	button* throw_button = gui_create_button();
	throw_button->width = graphics_projection_width / 20;
	throw_button->height = throw_button->width;
	throw_button->x = graphics_projection_width / 2 + (throw_button->width * 2) / 2;
	throw_button->y = throw_button->height / 8;
	throw_button->icon_img = action_throw_image;
	throw_button->bg_img = action_image;
	throw_button->hover_bg_img = action_hover_image;
	throw_button->click_callback = action_throw_mode;
	action_bar_buttons.push_back(throw_button);
}

mat4 create_model_matrix(vec3 pos, vec3 rot, vec3 scale)
{
	mat4 matrix = mat4(1.0f);
	matrix *= translate(matrix, pos);
	matrix *= rotate(matrix, 0.0f, rot);
	matrix *= glm::scale(matrix, scale * 4.0f);
	return matrix;
}

entity* selected_entity = NULL;

std::vector<vec3> path;

bool map_check_los(vec3 start, vec3 end)
{
	// the step of the world raytrace
	float accuracy = 0.25f;
	vec3 direction = glm::normalize(end - start);
	vec3 step = direction * accuracy;

	u32 timeout = 0;
	u32 max_distance = (u32) ceil(sqrt((float) terrain_max_x * terrain_max_x + (float) terrain_max_z * terrain_max_z) / accuracy);
	vec3 step_progress = start;
	vec3 last_block_pos = vec3(-1.0f);
	while(timeout++ < max_distance)
	{
		step_progress += step;

		vec3 next_step_block_pos = map_get_block_pos(step_progress);

		// only eval if this is a new block than last
		if(!map_pos_equal(next_step_block_pos, last_block_pos))
		{
			// if this block is cover, we do not have los
			if(map_get_cover_at_block(next_step_block_pos)) return false;

			path.push_back(next_step_block_pos);

			// if we reached the target, we have los
			if(map_pos_equal(next_step_block_pos, end)) return true;

			last_block_pos = next_step_block_pos;
		}
	}

	return false;
}

void update(float dt)
{
	camera_update(dt);
	input_update();

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

			if (current_action_mode == ACTION_MODE_SELECT_UNITS)
			{
				selected_entity = clicked_entity;
				path.clear();
			}
			else if (current_action_mode == ACTION_MODE_MOVE)
			{
				// we can only move to free blocks
				if (!clicked_entity)
				{
					selected_entity->pos = selected_block;
				}
				else
				{
					selected_entity = clicked_entity;
				}

				current_action_mode = ACTION_MODE_SELECT_UNITS;
			}
			else if(current_action_mode == ACTION_MODE_SHOOT)
			{
			    if(clicked_entity && clicked_entity != selected_entity)
				{
					path.clear();

					// maybe we only need to check along the opposite axis of the direction
					if(map_check_los(selected_entity->pos, clicked_entity->pos) || 
						map_check_los(vec3(selected_entity->pos.x + 1, selected_entity->pos.y, selected_entity->pos.z), clicked_entity->pos) ||
						map_check_los(vec3(selected_entity->pos.x - 1, selected_entity->pos.y, selected_entity->pos.z), clicked_entity->pos) ||
						map_check_los(vec3(selected_entity->pos.x, selected_entity->pos.y, selected_entity->pos.z + 1), clicked_entity->pos) ||
						map_check_los(vec3(selected_entity->pos.x, selected_entity->pos.y, selected_entity->pos.z - 1), clicked_entity->pos))
					{
						entity_health_change(clicked_entity, -6);
						current_action_mode = ACTION_MODE_SELECT_UNITS;
					}
				}
			}
			else if (current_action_mode == ACTION_MODE_THROW)
			{
				// @Todo: maybe we should have some abstract sense of "objects" that are on the map so we can remove them all together?
				for(u32 i = 0; i < entities.size(); i++)
				{
					entity* ent = entities[i];

					// euclidean distance
					float distance_squared = pow(abs(selected_block.x - ent->pos.x), 2) + pow(abs(selected_block.z - ent->pos.z), 2);

					if(distance_squared < 12)
					{
						entity_health_change(ent, -6);
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
			path.clear();
		}
	}
}

font* test_font = NULL;

void draw()
{
	if(test_font == NULL) test_font = load_font("res/Roboto-Black.ttf");

	// draw selected tile
	graphics_draw_mesh(cube_mesh, create_model_matrix(input_mouse_block_pos, vec3(1.0f), vec3(1.0f, 0.1f, 1.0f)));

	// @Remove: temp draw path
	for(u32 i = 0; i < path.size(); i++)
	{
		vec3 p = path[i];

		graphics_draw_mesh(cube_mesh, create_model_matrix(p, vec3(1.0f), vec3(1.0f, 0.05f, 1.0f)));
	}

	// @Todo: move to map_draw
	// draw cover
	for(u32 i = 0; i < cover_list.size(); i++)
	{
		cover* cov = cover_list[i];
		graphics_draw_mesh(cube_mesh, create_model_matrix(cov->pos, vec3(1.0f), vec3(1.0f, 2.0f, 1.0f)));
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
				graphics_draw_image(healthbar_image, create_model_matrix(vec3(ent->pos.x + 0.033333f, ent->pos.y + 2.038f, ent->pos.z + 0.5f), vec3(1.0f),
					vec3((0.5f - 0.1f / 3.0f) * (ent->health / (float)ent->max_health), 0.1285f, 1.0f)));
			}

			graphics_draw_image(healthbox_image, create_model_matrix(vec3(ent->pos.x, ent->pos.y + 2.0f, ent->pos.z + 0.5f), vec3(1.0f), healthbox_aspect * 0.5f));

			graphics_draw_mesh(ent->mesh, create_model_matrix(ent->pos, vec3(1.0f), vec3(1.0f)));
		}
	}

	// @Todo: move to map_draw
	// draw terrain
	graphics_draw_mesh(terrain_mesh, create_model_matrix(vec3(0.0f), vec3(1.0f), vec3(1.0f)));

	gui_draw_text("Hello! This is just a test.. I'm just saying, OK? k.", test_font, 0, 0);

	// draw action bar
	if(selected_entity)
	{
		// draw top action bar 
		if (current_action_mode != ACTION_MODE_SELECT_UNITS)
		{
			image img;

			if (current_action_mode == ACTION_MODE_SHOOT)
			{
				img = mode_text_shooting_image;
			}
			else if(current_action_mode == ACTION_MODE_THROW)
			{
				img = mode_text_throw_image;
			}
			else
			{	
				img = mode_text_move_image;
			}

			// bar bg
			u32 bar_width = graphics_projection_width / 4;
			u32 bar_height = graphics_projection_width / 25;
			gui_draw_image(action_bar_top_bg_image, graphics_projection_width / 2 - bar_width / 2, graphics_projection_height - bar_height, bar_width, bar_height);

			// bar text
			float scale = 0.5f;
			u32 text_width = (u32) (img.width * scale);
			u32 text_height = (u32) (img.height * scale);

			gui_draw_image(img, graphics_projection_width / 2 - text_width / 2, graphics_projection_height - text_height - (bar_height - text_height) / 2, 
				text_width, text_height);
		}

		// draw bottom action bar
		gui_draw_image(action_bar_bg_image, graphics_projection_width / 4, 0, graphics_projection_width / 2, graphics_projection_width / 30);

		for(u32 i = 0; i < action_bar_buttons.size(); i++)
		{
			gui_draw_button(*action_bar_buttons[i]);
		}
	}
}

