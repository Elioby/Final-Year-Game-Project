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
#include "mesh.h"
#include "shader.h"

using namespace glm;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define FOV 25.0f

mesh cube_mesh;
mesh terrain_mesh;

void load_meshes();
void load_terrain();
void draw(float dt);

int main()
{
	// Setup GLFW
	GLFWwindow* window;

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

	graphics_init(WINDOW_WIDTH, WINDOW_HEIGHT, FOV);

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
	cube_mesh = load_obj_mesh("res/newcube.obj");
}

u32 terrain_max_x = 10;
u32 terrain_max_z = 10;

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

float sinceStart = 0;

void draw(float dt)
{
	sinceStart += dt;
	mat4 transform_matrix = mat4(1.0f);
	transform_matrix *= translate(transform_matrix, vec3(0.0f, -0.7f, 0.0f));
	transform_matrix *= rotate(transform_matrix, 0.0f, vec3(0.0f, 1.0f, 0.0f));
	transform_matrix *= scale(transform_matrix, vec3(1.0f));
	draw_mesh(cube_mesh, transform_matrix);

	draw_mesh(terrain_mesh, transform_matrix);
}

