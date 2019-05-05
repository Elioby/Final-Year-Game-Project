#include "camera.h"

#include "graphics.h"
#include "input.h"
#include "window.h"

// @Todo: move to struct?
vec3 camera_pos_before_zoom = vec3(-6.0f, 15.0f, 30.0f);
vec3 camera_dir = vec3(0.0f);

float camera_pitch = -45.0f;
float camera_yaw = 0.0f;
float camera_fov = 65.0f;
float camera_zoom = -25.0f;

float camera_move_speed = 50.0f;
float camera_rotate_speed = 50.0f;
float camera_zoom_speed = 3.0f;

void camera_update(float dt)
{
	vec3 move_x = dt * camera_move_speed * vec3(camera_dir.x, 0.0f, camera_dir.z);

	// to move along z, we get the cross product of the normal move_x and up to get the perpendicular of them both which is 90deg clockwise of move_x
	vec3 move_z = dt * camera_move_speed * cross(normalize(vec3(camera_dir.x, 0.0f, camera_dir.z)), vec3(0.0f, 1.0f, 0.0f));

	if (glfwGetKey(window, GLFW_KEY_W)) camera_pos_before_zoom += move_x;
	if (glfwGetKey(window, GLFW_KEY_S)) camera_pos_before_zoom -= move_x;
	if (glfwGetKey(window, GLFW_KEY_D)) camera_pos_before_zoom += move_z;
	if (glfwGetKey(window, GLFW_KEY_A)) camera_pos_before_zoom -= move_z;

	if (glfwGetKey(window, GLFW_KEY_Q)) camera_yaw += dt * camera_rotate_speed;
	if (glfwGetKey(window, GLFW_KEY_E)) camera_yaw -= dt * camera_rotate_speed;

	if (input_mouse_wheel_delta_y != 0.0f)
	{
		float zoom_amount = input_mouse_wheel_delta_y;

		zoom_amount *= camera_zoom_speed;

		camera_zoom += zoom_amount;
	}

	// clamp infinitely close to -90 and +90
	camera_pitch = clamp(camera_pitch, -90.0f + EPSILON, 90.0f - EPSILON);

	vec3 cam_pos = camera_get_pos();
	camera_dir = vec3(cos(radians(camera_pitch)) * cos(radians(camera_yaw)), sin(radians(camera_pitch)), 
		cos(radians(camera_pitch)) * sin(radians(camera_yaw)));

	graphics_view_matrix = lookAt(cam_pos, cam_pos + camera_dir, vec3(0.0f, 1.0f, 0.0f));

	graphics_projection_matrix = perspective(radians(camera_fov), (float) graphics_projection_width / (float) graphics_projection_height, 0.1f, 10000.0f);

	bgfx_set_view_transform(0, &graphics_view_matrix, &graphics_projection_matrix);
}

vec3 camera_get_pos()
{
	return camera_pos_before_zoom + (camera_dir * camera_zoom);
}