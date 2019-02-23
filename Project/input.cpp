#include "input.h"

#include <stdio.h>

#include "graphics.h"
#include "window.h"
#include "raycast.h"
#include "map.h"

/* ------- Globals --------- */
double input_mouse_x, input_mouse_y = 0.0;
u8 input_mouse_button_left = INPUT_MOUSE_BUTTON_UP;
u8 input_mouse_button_right = INPUT_MOUSE_BUTTON_UP;
float input_mouse_wheel_delta_y = 0.0;
vec3 input_mouse_ray = vec3(0.0f);
vec3 input_mouse_block_pos = vec3(0.0f);

/* ------- Locals --------- */
vec3 calculate_mouse_ray();
void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void input_init()
{
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
	glfwSetScrollCallback(window, glfw_scroll_callback);
}

void input_update()
{
	glfwGetCursorPos(window, &input_mouse_x, &input_mouse_y);

	// turn higher y into higher screen position
	input_mouse_y = graphics_projection_height - input_mouse_y;

	input_mouse_ray = calculate_mouse_ray();

	vec3 intersection = ray_plane_intersection(graphics_camera_pos, input_mouse_ray, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

	if(!map_pos_equal(intersection, vec3(-1.0f)))
	{
		u32 clamped_x = (u32) clamp(floor(intersection.x), 0.0f, (float) terrain_max_x - 1);
		u32 clamped_z = (u32) clamp(floor(intersection.z), 0.0f, (float) terrain_max_z - 1);

		// @Volatile: no account for y?
		input_mouse_block_pos = vec3(clamped_x, 0.0f, clamped_z);
	}
}

void input_end_frame()
{
	input_mouse_wheel_delta_y = 0.0f;

	// next frame, the button will not have started to be up or down (unless it happens to happen next time we poll events)
	if(input_mouse_button_left == INPUT_MOUSE_BUTTON_UP_START) input_mouse_button_left = INPUT_MOUSE_BUTTON_UP;
	if(input_mouse_button_left == INPUT_MOUSE_BUTTON_DOWN_START) input_mouse_button_left = INPUT_MOUSE_BUTTON_DOWN;

	if (input_mouse_button_right == INPUT_MOUSE_BUTTON_UP_START) input_mouse_button_right = INPUT_MOUSE_BUTTON_UP;
	if (input_mouse_button_right == INPUT_MOUSE_BUTTON_DOWN_START) input_mouse_button_right = INPUT_MOUSE_BUTTON_DOWN;
}

vec3 calculate_mouse_ray()
{
	float x = (2.0f * input_mouse_x) / (float)graphics_projection_width - 1.0f;
	float y = (2.0f * input_mouse_y) / (float)graphics_projection_height - 1.0f;

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
	return normalize(vec3(ray_world.x, ray_world.y, ray_world.z));
}

void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}

void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if(action == GLFW_PRESS) input_mouse_button_left = INPUT_MOUSE_BUTTON_DOWN_START;
		if(action == GLFW_RELEASE) input_mouse_button_left = INPUT_MOUSE_BUTTON_UP_START;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS) input_mouse_button_right = INPUT_MOUSE_BUTTON_DOWN_START;
		if (action == GLFW_RELEASE) input_mouse_button_right = INPUT_MOUSE_BUTTON_UP_START;
	}
}

void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	input_mouse_wheel_delta_y = yoffset;
}