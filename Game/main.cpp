#include <stdio.h>

#include <glm/glm.hpp>

#include <bgfx/c99/bgfx.h>

#include "general.h"
#include "graphics.h"
#include "input.h"
#include "mesh.h"
#include "shader.h"
#include "raycast.h"
#include "gui.h"
#include "image.h"
#include "entity.h"
#include "assets.h"
#include "map.h"
#include "window.h"

void draw();
void update(float dt);
void camera_update(float dt);

int main()
{
	window_init();
	graphics_init(WINDOW_WIDTH, WINDOW_HEIGHT);
	input_init(window);
	gui_init();
	assets_init();
	map_init();

	float lastTime = 0;
	float time, dt;

	while(!glfwWindowShouldClose(window))
	{
		time = (float) glfwGetTime();
		dt = time - lastTime;
		lastTime = time;

		glfwPollEvents();
		input_update();

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx_touch(0);

		bgfx_dbg_text_clear(0, false);

		bgfx_dbg_text_printf(0, 0, 0x0f, "Last frame time: %.2fms, FPS: %.0f", dt * 1000.0f, 1 / dt);

		update(dt);
		draw();

		bgfx_frame(false);

		input_end_frame();
	}

	bgfx_shutdown();
	glfwTerminate();

	return 0;
}

mat4 create_model_matrix(vec3 pos, vec3 rot, vec3 scale)
{
	mat4 matrix = mat4(1.0f);
	matrix *= translate(matrix, pos);
	matrix *= rotate(matrix, 0.0f, rot);
	matrix *= glm::scale(matrix, scale * 4.0f);
	return matrix;
}

// @Todo: move!!
vec3 last_known_intersection = vec3(0.0f, 0.0f, 0.0f);
vec3 mouse_block_pos;

void update(float dt)
{
	camera_update(dt);

	vec3* intersection = ray_plane_intersection(graphics_camera_pos, input_mouse_ray, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

	if (intersection != NULL) last_known_intersection = *intersection;

	u32 clamped_x = (u32) clamp(floor(last_known_intersection.x), 0.0f, (float) terrain_max_x - 1);
	u32 clamped_z = (u32) clamp(floor(last_known_intersection.z), 0.0f, (float) terrain_max_z - 1);

	mouse_block_pos = vec3(clamped_x, last_known_intersection.y, clamped_z);
}

void draw()
{
	// draw selected tile
	graphics_draw_mesh(cube_mesh, create_model_matrix(mouse_block_pos, vec3(1.0f), vec3(1.0f, 0.1f, 1.0f)));

	// draw entites and healthbars
	for (u32 i = 0; i < entities.size(); i++)
	{
		entity ent = entities[i];

		vec3 healthbox_aspect = vec3(1.0f, 1.0f / 3.0f, 1.0f);
		graphics_draw_image(healthbox_image, create_model_matrix(vec3(ent.pos.x, ent.pos.y + 2.0f, ent.pos.z + 0.5f), vec3(1.0f), healthbox_aspect * 0.5f));

		graphics_draw_mesh(ent.mesh, create_model_matrix(ent.pos, vec3(1.0f), vec3(1.0f)));
	}

	// draw terrain
	graphics_draw_mesh(terrain_mesh, create_model_matrix(vec3(0.0f), vec3(1.0f), vec3(1.0f)));

	// draw action bar
	gui_draw_image(action_bar_bg_image, graphics_projection_width / 4, 0, graphics_projection_width / 2, graphics_projection_width / 30);

	// draw action bar buttons
}

