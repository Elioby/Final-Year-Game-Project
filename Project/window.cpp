#include "window.h"

#include <assert.h>

#include <bgfx/c99/platform.h>

GLFWwindow* window;

void window_init()
{
	// Setup GLFW
	assert(glfwInit() && "GLFW init failed");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Game Window", NULL, NULL);

	assert(window && "Failed to create window");

	glfwMakeContextCurrent(window);

	bgfx_platform_data_t platform_data = {};

	// @Todo: add x11, wayland, mir support for windows?
	#ifdef _WIN32
		platform_data.nwh = glfwGetWin32Window(window);
	#elif __APPLE__
		platformData.nwh = glfwGetCocoaWindow(mWindow);
	#else
		assert(false && "Unsupported platform");
	#endif

	bgfx_set_platform_data(&platform_data);
}

bool window_error_message_box(char* message)
{
	// @Todo: add x11, wayland, mir support for windows?
	#ifdef _WIN32
		MessageBox(NULL, message, "Fatal Error", MB_SYSTEMMODAL | MB_ICONERROR);
	#elif __APPLE__
		StandardAlert(kAlertStopAlert, message, null, null, kAlertStdAlertOKButton);
	#else
		return false;
	#endif

	return true;
}