#pragma once

#include "general.h"

vec3 physics_raycast_from_screen_location(float screen_x, float screen_y);
vec3 physics_plane_intersection_with_ray(vec3 ray_origin, vec3 ray_direction, vec3 plane_origin, vec3 plane_direction);