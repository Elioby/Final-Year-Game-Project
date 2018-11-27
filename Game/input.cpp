#include "input.h"

/* ------- Globals --------- */
double input_mouse_x, input_mouse_y = 0;

/* ------- Locals --------- */
GLFWwindow* main_window;

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void input_init(GLFWwindow* window)
{
	main_window = window;

	glfwSetKeyCallback(main_window, glfw_key_callback);
}

void input_update()
{
	glfwGetCursorPos(main_window, &input_mouse_x, &input_mouse_y);
}

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}