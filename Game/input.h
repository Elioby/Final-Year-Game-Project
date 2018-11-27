#pragma once

#include <GLFW\glfw3.h>

extern double input_mouse_x, input_mouse_y;

void input_init(GLFWwindow* window);
void input_update();