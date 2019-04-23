#pragma once

#include <float.h>
#include <stdio.h>
#include <string.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <bgfx/c99/bgfx.h>

#include "debug.h"
#include "math.h"

#define EPSILON 0.001f

#define stack_array_length(arr) sizeof(arr) / sizeof(arr[0])

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

using glm::radians;
using glm::perspective;
using glm::clamp;
using glm::cross;