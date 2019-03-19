#pragma once

// @Todo: add x11, wayland, mir support for windows?
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

extern GLFWwindow* window;

void window_init();

bool window_error_message_box(char* message);