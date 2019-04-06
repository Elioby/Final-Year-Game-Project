#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "general.h"
#include "mesh.h"
#include "image.h"

#define GRAPHICS_USE_VSYNC true

/* ------- Globals --------- */
extern bgfx_renderer_type_t graphics_renderer_type;
extern bool graphics_debug_draw_normals;

extern mat4 graphics_view_matrix;
extern mat4 graphics_projection_matrix;

// The resolution we render at, not nessisarily the window width and height
extern u32 graphics_projection_width;
extern u32 graphics_projection_height;

extern bgfx_uniform_handle_t texture_sampler;

void graphics_init(int w_width, int w_height);

void graphics_draw_mesh(mesh* mesh, mat4 transform_matrix);
void graphics_draw_mesh(mesh* mesh, mat4 transform_matrix, vec4 color);
void graphics_draw_image(image* image, mat4 transform_matrix);

mat4 graphics_create_model_matrix(vec3 pos, float rot, vec3 rot_axis, vec3 scale);

#endif