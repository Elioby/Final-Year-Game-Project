#pragma once

#include "general.h"

extern vec3 camera_dir;

extern float camera_pitch;
extern float camera_yaw;
extern float camera_fov;
extern float camera_zoom;

extern float camera_move_speed;
extern float camera_rotate_speed;

void camera_update(float dt);

vec3 camera_get_pos();