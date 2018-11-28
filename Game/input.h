#pragma once

#include <GLFW\glfw3.h>

#include "general.h"

#define INPUT_MOUSE_BUTTON_UP 0
#define INPUT_MOUSE_BUTTON_DOWN 1

// the mouse button came up or down this frame (only happens for 1 frame)
#define INPUT_MOUSE_BUTTON_UP_START 2
#define INPUT_MOUSE_BUTTON_DOWN_START 3

extern double input_mouse_x, input_mouse_y;
extern u8 input_mouse_button_left;
extern float input_mouse_wheel_delta_y;

extern vec3 input_mouse_ray;

void input_init(GLFWwindow* window);
void input_update();
void input_end_frame();