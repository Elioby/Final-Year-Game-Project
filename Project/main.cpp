#include <stdio.h>

#include <glm/glm.hpp>

#include <bgfx/c99/bgfx.h>

#include "general.h"
#include "window.h"
#include "graphics.h"
#include "input.h"
#include "gui.h"
#include "asset_manager.h"
#include "map.h"
#include "entity.h"
#include "font.h"
#include "dynstr.h"
#include "actionbar.h"
#include "action.h"

void draw();
void update(float dt);
void camera_update(float dt);
void turn_init();

void turn_end();
// @Todo: temp
button* b;

void button_end_turn_all()
{
	turn_end();
}

int main()
{
	window_init();
	graphics_init(WINDOW_WIDTH, WINDOW_HEIGHT);
	input_init();
	gui_init();
	asset_manager_init();
	map_init();
	actionbar_init();
	turn_init();
	shader_init();

	b = gui_create_button();
	b->width = 100;
	b->height = 100;
	b->bg_img = asset_manager_get_image("action_button");
	b->hover_bg_img = asset_manager_get_image("action_button_hover");
	b->click_callback = button_end_turn_all;

	float lastTime = 0;
	float time, dt;

	shader_set_tint_uniform(vec4(1.0f, 0.0f, 0.0f, 1.0f));

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
		gui_end_frame();
	}

	bgfx_shutdown();
	glfwTerminate();

	return 0;
}

u32 turn_number = 1;
team turn_team;

bool team_is_turn_over(team team)
{
	u32 team_entities = 0;
	u32 team_entities_ended = 0;

	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];

		if(entity->team == team)
		{
			team_entities++;

			if(entity->ap <= 0) team_entities_ended++;
		}
	}

	return team_entities == team_entities_ended;
}

char* team_get_name(team team)
{
	switch(team)
	{
		case TEAM_FRIENDLY: return "Friendly";
		case TEAM_ENEMY: return "Enemy";
	}

	return "unknown team";
}

void turn_start(team team);

void turn_init()
{
	turn_start(TEAM_FRIENDLY);
}

void turn_start(team team)
{
	turn_team = team;

	// reset ap
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];

		if(entity->team == team) entity->ap = entity->max_ap;
	}

	actionbar_combatlog_add("%s team's turn started", team_get_name(team));
}

void turn_end()
{
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];

		if(entity->team == turn_team) entity->ap = 0;
	}

	turn_number++;

	actionbar_combatlog_add("%s team's turn ended", team_get_name(turn_team));

	team new_team;

	if(turn_team == TEAM_FRIENDLY) new_team = TEAM_ENEMY;
	else new_team = TEAM_FRIENDLY;

	turn_start(new_team);
}

void update(float dt)
{
	camera_update(dt);
	input_update();

	gui_update();

	entity_update();
	action_update();
	actionbar_update(dt);

	// turn update
	if(team_is_turn_over(turn_team))
	{
		turn_end();
	}
}

void draw()
{
	b->x = graphics_projection_width - b->width - 30;
	b->y = graphics_projection_height - b->height - 30;
	gui_draw_button(b);

	// @Debug: debug los
	for(u32 i = 0; i < map_debug_los.size(); i++)
	{
		graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(map_debug_los[i], 0.0f, vec3(1.0f), vec3(1.0f, 0.25f, 1.0f)));
	}

	// draw selected tile
	graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(input_mouse_block_pos, 0.0f, vec3(1.0f), vec3(1.0f, 0.1f, 1.0f)));

	// draw selected entity
	if(selected_entity)
	{
		graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(selected_entity->pos, radians(0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.05f, 1.0f)));
	}

	// @Todo: move to map_draw
	// draw cover
	for(u32 i = 0; i < cover_list.size(); i++)
	{
		cover* cov = cover_list[i];
		graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(cov->pos, 0.0f, vec3(1.0f), vec3(1.0f, 2.0f, 1.0f)));
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
				image* img;
				if(ent->team == TEAM_ENEMY) img = asset_manager_get_image("enemy_healthbar");
				else img = asset_manager_get_image("friendly_healthbar");

				graphics_draw_image(img, graphics_create_model_matrix(vec3(ent->pos.x + 0.033333f, ent->pos.y + 2.038f, ent->pos.z + 0.5f), 0.0f, vec3(1.0f),
					vec3((0.5f - 0.1f / 3.0f) * (ent->health / (float)ent->max_health), 0.1285f, 1.0f)));
			}

			graphics_draw_image(asset_manager_get_image("healthbox"), graphics_create_model_matrix(vec3(ent->pos.x, ent->pos.y + 2.0f, ent->pos.z + 0.5f), 0.0f, vec3(1.0f), healthbox_aspect * 0.5f));

			graphics_draw_mesh(ent->mesh, graphics_create_model_matrix(ent->pos, 0.0f, vec3(1.0f), vec3(1.0f)));
		}
	}

	// @Todo: move to map_draw
	// draw terrain
	graphics_draw_mesh(asset_manager_get_mesh("terrain"), graphics_create_model_matrix(vec3(0.0f), 0.0f, vec3(1.0f), vec3(1.0f)));

	// draw action bar
	actionbar_draw();
}

