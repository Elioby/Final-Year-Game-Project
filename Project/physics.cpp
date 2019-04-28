#include "physics.h"

#include <stdio.h>

#include "graphics.h"

vec3 physics_plane_intersection_with_ray(vec3 ray_origin, vec3 ray_direction, vec3 plane_origin, vec3 plane_direction)
{
	float denominator = glm::dot(plane_direction, ray_direction);

	if(abs(denominator) > EPSILON) // stop divide by 0
	{
		// the distance along the ray at which it intersects with the plane
		float t = glm::dot(plane_origin - ray_origin, plane_direction) / denominator;

		if (t > EPSILON) // only if the intersection is in front of the ray
		{
			// get the point at t distance along ray_direction (and offset by the ray origin)
			vec3 plane_intersection = (ray_direction * t) + ray_origin;

			return plane_intersection;
		}
	}

	return vec3(-1.0f);
}

vec3 physics_raycast_from_screen_location(float screen_x, float screen_y)
{
	// normalize the position into graphics coords (-1.0 to 1.0, -1.0 to 1.0)
	float x = (2.0f * (float) screen_x) / (float) graphics_projection_width - 1.0f;
	float y = (2.0f * (float) screen_y) / (float) graphics_projection_height - 1.0f;

	vec3 ray_norm = vec3(x, y, 1.0f);

	// move into clip coords and point the vector out of the screen (+1 on z)
	vec4 ray_clip = vec4(ray_norm.x, ray_norm.y, 1.0f, 0.0f);

	// move into eye coords
	vec4 ray_eye = inverse(graphics_projection_matrix) * ray_clip;
	ray_eye.z = -1.0f;
	ray_eye.w = 0.0f;

	// move into world coords by mulitplying by the inverse of the view matrix (and norm because we only need a direction vector)
	vec4 ray_world = inverse(graphics_view_matrix) * ray_eye;
	return normalize(vec3(ray_world.x, ray_world.y, ray_world.z));
}