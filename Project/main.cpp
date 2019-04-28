#include <stdio.h>

#include <glm/glm.hpp>

#include <bgfx/c99/bgfx.h>

#include <libmorton/morton.h>

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
#include "hashtable.h"
#include "turn.h"
#include "board_eval.h"
#include "dynarray.h"

void draw();
void update(float dt);
void camera_update(float dt);
void action_init();

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
	action_init();
	actionbar_init();
	shader_init();

	debug_print_total_allocated();

	b = gui_create_button();
	b->width = 100;
	b->height = 100;
	b->bg_img = asset_manager_get_image("action_button");
	b->hover_bg_img = asset_manager_get_image("action_button_hover");
	b->click_callback = button_end_turn_all;

	// start the first turn straight away
	turn_start(TEAM_FRIENDLY);

	float lastTime = 0;
	float time = 0, dt = 0;

	shader_set_tint_uniform(vec4(1.0f, 0.0f, 0.0f, 1.0f));

	while(!glfwWindowShouldClose(window))
	{
		dt = time - lastTime;
		lastTime = time;

		time = (float) glfwGetTime();

		glfwPollEvents();
		update(dt);

		bgfx_dbg_text_clear(0, false);

		bgfx_dbg_text_printf(0, 0, 0x0f, "Last frame time: %.2fms, FPS: %.0f", dt * 1000.0f, 1 / dt);
		bgfx_dbg_text_printf(0, 1, 0x0f, "Mouse block: (%f, %f)", input_mouse_block_pos.x, input_mouse_block_pos.z);

		draw();

		input_end_frame();
		gui_end_frame();

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx_touch(0);

		graphics_end_frame(dt);
	}

	bgfx_shutdown();
	glfwTerminate();

	return 0;
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
	if(turn_is_complete(turn_team))
	{
		turn_end();
	}
}

void draw()
{
	b->x = graphics_projection_width - b->width - 30;
	b->y = graphics_projection_height - b->height - 30;
	gui_draw_button(b);

	// draw selected tile
	graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(input_mouse_block_pos, 0.0f, vec3(1.0f), vec3(1.0f, 0.1f, 1.0f)));

	// draw selected entity
	if(selected_entity)
	{
		graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(selected_entity->pos, radians(0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.05f, 1.0f)));
	}

	map_draw();

	// draw action bar
	actionbar_draw();
}

