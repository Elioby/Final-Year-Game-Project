// @Todo: add x11, wayland, mir support for windows?
#ifdef _WIN32
	#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#ifdef __APPLE__
	#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <stdio.h>
#include <assert.h>

#include <vector>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>

#include <bgfx/c99/bgfx.h>
#include <bgfx/c99/platform.h>

#include "general.h"
#include "graphics.h"
#include "input.h"
#include "mesh.h"
#include "shader.h"
#include "raycast.h"
#include "gui.h"
#include "image.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

mesh cube_mesh;
mesh robot_mesh;
mesh terrain_mesh;
image shoot_image;
image test_image;

void load_meshes();
void load_images();
void load_terrain();
void draw();
void update(float dt);
void camera_update(float dt);

GLFWwindow* window;

int main()
{
	// Setup GLFW
	assert(glfwInit());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Game Window", NULL, NULL);

	assert(window);

	glfwMakeContextCurrent(window);

	bgfx_platform_data_t platform_data = {};

	// @Todo: add x11, wayland, mir support for windows?
	#ifdef _WIN32
		platform_data.nwh = glfwGetWin32Window(window);
	#endif

	#ifdef __APPLE__
		platformData.nwh = glfwGetCocoaWindow(mWindow);
	#endif

	bgfx_set_platform_data(&platform_data);

	graphics_init(WINDOW_WIDTH, WINDOW_HEIGHT);
	input_init(window);
	gui_init();

	float lastTime = 0;
	float time, dt;

	load_shaders();
	load_terrain();
	load_meshes();
	load_images();

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

void load_meshes()
{
	cube_mesh = load_obj_mesh("res/mesh/cube.obj");
	robot_mesh = load_obj_mesh("res/mesh/robot.obj");
}

void load_images()
{
	shoot_image = load_image("res/horse.png");
	test_image = load_image("res/test.png");
}

u32 terrain_max_x = 25;
u32 terrain_max_z = 25;

void load_terrain()
{
	// x * y squares, 2 triangles per square, 3 vertices per triangle
	u32 vertex_count = terrain_max_x * terrain_max_z * 3 * 2;
	pos_normal_vertex* vertices = (pos_normal_vertex*) malloc(vertex_count * sizeof(pos_normal_vertex));

	u32 i = 0;
	u32 up_normal = pack_vec3_into_u32(vec3(0.0f, 1.0f, 0.0f));

	for(float x = 0; x < terrain_max_x; x++)
	{
		for(float z = 0; z < terrain_max_z; z++)
		{
			vertices[i++] = { x, 0, z, up_normal };
			vertices[i++] = { x + 1.0f, 0, z + 1.0f, up_normal };
			vertices[i++] = { x + 1.0f, 0, z, up_normal };
			vertices[i++] = { x, 0, z, up_normal };
			vertices[i++] = { x, 0, z + 1.0f, up_normal };
			vertices[i++] = { x + 1.0f, 0, z + 1.0f, up_normal };
		}
	}

	terrain_mesh = create_mesh(vertices, vertex_count);
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
float time;

vec3 last_known_intersection = vec3(0.0f, 0.0f, 0.0f);
vec3 move_to;
vec3 pos = vec3(0, 0, 0.0f);
bool moving = false;
vec3 mouse_block_pos;

void update(float dt)
{
	camera_update(dt);

	vec3* intersection = ray_plane_intersection(graphics_camera_pos, input_mouse_ray, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

	if (intersection != NULL) last_known_intersection = *intersection;

	u32 clamped_x = (u32)clamp(floor(last_known_intersection.x), 0.0f, (float)terrain_max_x - 1);
	u32 clamped_z = (u32)clamp(floor(last_known_intersection.z), 0.0f, (float)terrain_max_z - 1);

	mouse_block_pos = vec3(clamped_x, last_known_intersection.y, clamped_z);

	// move only if we click, if we actually clicked on the plane, and we're not still moving
	if (input_mouse_button_left == INPUT_MOUSE_BUTTON_UP_START && intersection != NULL && !moving)
	{
		move_to = mouse_block_pos;
		moving = true;
	}

	// @Cleanup: this is not the best!
	if (moving)
	{
		if (pos.x < move_to.x)
		{
			pos.x += clamp(10 * dt, 0.0f, move_to.x - pos.x);
		}
		else if (pos.x > move_to.x)
		{
			pos.x -= clamp(10 * dt, 0.0f, abs(move_to.x - pos.x));
		}
		else if (pos.z < move_to.z)
		{
			pos.z += clamp(10 * dt, 0.0f, move_to.z - pos.z);
		}
		else if (pos.z > move_to.z)
		{
			pos.z -= clamp(10 * dt, 0.0f, abs(move_to.z - pos.z));
		}
		else
		{
			// we reached the goal
			moving = false;
		}
	}
}

void camera_update(float dt)
{
	// zooming and moving gets exponentially slower the closer in you get
	float zoom_speed_modifier = clamp((graphics_camera_zoom * graphics_camera_zoom) / 80.0f, 0.5f, 100.0f);

	vec3 move_x = dt * graphics_camera_move_speed / zoom_speed_modifier * vec3(graphics_camera_dir.x, 0.0f, graphics_camera_dir.z);

	// to move along z, we get the cross product of the normal move_x and up to get the perpendicular of them both which is 90deg clockwise of move_x
	vec3 move_z = dt * graphics_camera_move_speed / zoom_speed_modifier * cross(normalize(vec3(graphics_camera_dir.x, 0.0f, graphics_camera_dir.z)), vec3(0.0f, 1.0f, 0.0f));

	if (glfwGetKey(window, GLFW_KEY_W)) graphics_camera_pos += move_x;
	if (glfwGetKey(window, GLFW_KEY_S)) graphics_camera_pos -= move_x;
	if (glfwGetKey(window, GLFW_KEY_D)) graphics_camera_pos += move_z;
	if (glfwGetKey(window, GLFW_KEY_A)) graphics_camera_pos -= move_z;

	if (glfwGetKey(window, GLFW_KEY_Q)) graphics_camera_yaw += dt * graphics_camera_rotate_speed;
	if (glfwGetKey(window, GLFW_KEY_E)) graphics_camera_yaw -= dt * graphics_camera_rotate_speed;

	if (input_mouse_wheel_delta_y != 0.0f)
	{
		float zoom_amount;

		if (graphics_camera_zoom > 0.0f) zoom_amount = input_mouse_wheel_delta_y / zoom_speed_modifier;
		else zoom_amount = input_mouse_wheel_delta_y;

		graphics_camera_zoom += zoom_amount;
		graphics_camera_pos += graphics_camera_dir * zoom_amount;
	}

	graphics_update_camera();
}

gui_button button = { 0, 0, 100, 100 };

void buttoncb()
{
	printf("Button press!\n");
}

void draw()
{
	graphics_draw_mesh(cube_mesh, create_model_matrix(mouse_block_pos, vec3(1.0f), vec3(1.0f, 0.1f, 1.0f)));

	graphics_draw_mesh(robot_mesh, create_model_matrix(pos, vec3(1.0f), vec3(1.0f)));

	graphics_draw_mesh(terrain_mesh, create_model_matrix(vec3(0.0f), vec3(1.0f), vec3(1.0f)));

	button.img = shoot_image;
	button.hover_img = test_image;
	button.has_hover = true;

	button.click_callback = &buttoncb;

	gui_draw_button(button);
}

