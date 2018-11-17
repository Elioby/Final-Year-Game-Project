#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "general.h"
#include "mesh.h"

/* ------- Globals --------- */
extern bgfx_renderer_type_t renderer_type;
extern bool graphics_debug_draw_normals;

void graphics_init(int window_width, int window_height, float fov);

void draw_mesh(mesh mesh, mat4 transform_matrix);

#endif