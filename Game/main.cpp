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

#include "graphics.h"
#include "input.h"
#include "mesh.h"
#include "shader.h"
#include "raycast.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

mesh cube_mesh;
mesh robot_mesh;
mesh terrain_mesh;

void load_meshes();
void load_terrain();
void draw(float dt);

int main()
{
	// Setup GLFW
	assert(glfwInit());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Game Window", NULL, NULL);

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

	float lastTime = 0;
	float time, dt;

	load_shaders();
	load_terrain();
	load_meshes();

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

		draw(dt);

		bgfx_frame(false);
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

// @Todo: cstdlib rand is pretty bad
i32 next_random_i32(i32 min, i32 max)
{
	return (i32) (((float) rand() / (float) RAND_MAX) * max + min);
}

float time;

vec3 last_known_intersection = vec3(0.0f, 0.0f, 0.0f);

void draw(float dt)
{
	vec3 plane_origin = vec3(0.0f, 0.0f, 0.0f);

	float x = (2.0f * input_mouse_x) / (float) WINDOW_WIDTH - 1.0f;
	float y = -((2.0f * input_mouse_y) / (float) WINDOW_HEIGHT - 1.0f);

	// normalize the position into graphics coords (-1.0 to 1.0, -1.0 to 1.0)
	vec3 ray_norm = vec3(x, y, 1.0f);

	// move into clip coords and point the vector into the screen (-1 on z)
	vec4 ray_clip = vec4(ray_norm.x, ray_norm.y, -1.0f, 1.0f);

	// move into eye coords
	vec4 ray_eye = inverse(graphics_projection_matrix) * ray_clip;
	ray_eye.z = -1.0f;
	ray_eye.w = 0.0f;

	// move into world coords by mulitplying by the inverse of the view matrix (and norm because we only need a direction vector)
	vec4 ray_world = inverse(graphics_view_matrix) * ray_eye;
	vec3 ray_world_norm = normalize(vec3(ray_world.x, ray_world.y, ray_world.z));

	vec3* intersection = ray_plane_intersection(graphics_camera_pos, ray_world_norm, plane_origin, vec3(0.0f, 1.0f, 0.0f));

	if (intersection != NULL) last_known_intersection = vec4(*intersection, 1.0f);

	graphics_draw_mesh(cube_mesh, create_model_matrix(vec3(clamp(floor(last_known_intersection.x), plane_origin.x, (float) terrain_max_x + plane_origin.x - 1),
		floor(last_known_intersection.y), clamp(floor(last_known_intersection.z), plane_origin.z, (float) terrain_max_z + plane_origin.z - 1)), vec3(1.0f), vec3(1.0f, 0.1f, 1.0f)));

	graphics_draw_mesh(robot_mesh, create_model_matrix(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f), vec3(1.0f)));

	graphics_draw_mesh(terrain_mesh, create_model_matrix(plane_origin, vec3(1.0f), vec3(1.0f)));

	graphics_update_camera();
}

