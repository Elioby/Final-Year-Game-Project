#include "raycast.h"

#include <stdio.h>

#include "graphics.h"

vec3* ray_plane_intersection(vec3 ray_origin, vec3 ray_direction, vec3 plane_origin, vec3 plane_direction)
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

			return &plane_intersection;
		}
	}

	return NULL;
}