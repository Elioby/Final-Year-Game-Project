#include "camera.h"

#include "graphics.h"
#include "input.h"
#include "window.h"

void camera_update(float dt)
{
	// zooming and moving gets exponentially slower the closer in you get

	vec3 move_x = dt * graphics_camera_move_speed * vec3(graphics_camera_dir.x, 0.0f, graphics_camera_dir.z);

	// to move along z, we get the cross product of the normal move_x and up to get the perpendicular of them both which is 90deg clockwise of move_x
	vec3 move_z = dt * graphics_camera_move_speed * cross(normalize(vec3(graphics_camera_dir.x, 0.0f, graphics_camera_dir.z)), vec3(0.0f, 1.0f, 0.0f));

	if (glfwGetKey(window, GLFW_KEY_W)) graphics_camera_pos += move_x;
	if (glfwGetKey(window, GLFW_KEY_S)) graphics_camera_pos -= move_x;
	if (glfwGetKey(window, GLFW_KEY_D)) graphics_camera_pos += move_z;
	if (glfwGetKey(window, GLFW_KEY_A)) graphics_camera_pos -= move_z;

	if (glfwGetKey(window, GLFW_KEY_Q)) graphics_camera_yaw += dt * graphics_camera_rotate_speed;
	if (glfwGetKey(window, GLFW_KEY_E)) graphics_camera_yaw -= dt * graphics_camera_rotate_speed;

	if (input_mouse_wheel_delta_y != 0.0f)
	{
		float zoom_amount;

		if (graphics_camera_zoom > 0.0f) zoom_amount = input_mouse_wheel_delta_y;
		else zoom_amount = input_mouse_wheel_delta_y;

		graphics_camera_zoom += zoom_amount;
		graphics_camera_pos += graphics_camera_dir * zoom_amount;
	}

	graphics_update_camera();
}