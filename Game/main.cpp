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
#include "model.h"
#include "shader.h"

using namespace glm;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define FOV 15.0f

model cube_model;

void load_models();
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
	load_models();

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

void load_models()
{
	cube_model = load_obj_model("res/cube.obj");
}

float sinceStart = 0;

void draw(float dt)
{
	sinceStart += dt;
	mat4 model_matrix = mat4(1.0f);
	model_matrix = rotate(model_matrix, radians(sinceStart * 70), vec3(0.0f, 1.0f, 0.0f));
	bgfx_set_transform(&model_matrix, 1);
	draw_model(cube_model);
}

