#pragma once

#include <GLFW\glfw3.h>

#include "general.h"

extern double input_mouse_x, input_mouse_y;

extern vec3 input_mouse_ray;

void input_init(GLFWwindow* window);
void input_update();