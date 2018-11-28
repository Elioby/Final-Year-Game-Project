#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "general.h"
#include "mesh.h"

/* ------- Globals --------- */
extern bgfx_renderer_type_t graphics_renderer_type;
extern bool graphics_debug_draw_normals;

extern mat4 graphics_view_matrix;
extern mat4 graphics_projection_matrix;

// @Todo: this needs to be better!
extern vec3 graphics_camera_pos;
extern vec3 graphics_camera_dir;

extern float graphics_camera_pitch;
extern float graphics_camera_yaw;
extern float graphics_camera_fov;
extern float graphics_camera_zoom;

extern float graphics_camera_move_speed;
extern float graphics_camera_rotate_speed;

// The resolution we render at, not nessisarily the window width and height
extern u32 graphics_projection_width;
extern u32 graphics_projection_height;

void graphics_init(int w_width, int w_height);
void graphics_update_camera();

void graphics_draw_mesh(mesh mesh, mat4 transform_matrix);

#endif