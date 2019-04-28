#include "graphics.h"

#include <stdio.h>

#include <vector>

#include "asset_manager.h"
#include "mesh.h"

/* ------- Globals --------- */
bgfx_renderer_type_t graphics_renderer_type;

mat4 graphics_view_matrix;
mat4 graphics_projection_matrix;

u32 graphics_projection_width;
u32 graphics_projection_height;

bgfx_uniform_handle_t graphics_texture_sampler;

bgfx_uniform_handle_t graphics_tint_color;

void graphics_init(int window_width, int window_height)
{
	bgfx_init_t init;
	bgfx_init_ctor(&init);
	init.type = BGFX_RENDERER_TYPE_OPENGL;

	bgfx_init(&init);

	graphics_tint_color = bgfx_create_uniform("tint_color", BGFX_UNIFORM_TYPE_VEC4, 1);
	graphics_texture_sampler = bgfx_create_uniform("textureSampler", BGFX_UNIFORM_TYPE_SAMPLER, 1);

	u32 flags = BGFX_RESET_MSAA_X16;

	if(GRAPHICS_USE_VSYNC) flags |= BGFX_RESET_VSYNC;

	bgfx_reset(window_width, window_height, flags, init.resolution.format);

	bgfx_set_debug(BGFX_DEBUG_TEXT);

	graphics_renderer_type = bgfx_get_renderer_type();

	bgfx_set_view_clear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

	// setup view and projection matrix
	graphics_projection_width = window_width;
	graphics_projection_height = window_height;

	bgfx_set_view_rect(0, 0, 0, window_width, window_height);
}

void graphics_end_frame(float dt)
{
	bgfx_frame(false);
}

void graphics_draw_mesh(mesh* mesh, mat4 transform_matrix, vec4 color)
{
	bgfx_set_vertex_buffer(0, mesh->vb_handle, 0, mesh->vertex_count);
	bgfx_set_index_buffer(mesh->idb_handle, 0, mesh->index_count);

	bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA, 0);

	bgfx_set_transform(&transform_matrix, 1);

	bgfx_set_uniform(graphics_tint_color, &color, 1);

	bgfx_submit(0, asset_manager_get_shader("diffuse")->handle, 0, false);
}

void graphics_draw_mesh(mesh* mesh, mat4 transform_matrix)
{
	graphics_draw_mesh(mesh, transform_matrix, vec4(0.8f, 0.8f, 0.8f, 1.0f));
}

void graphics_draw_image(image* image, mat4 transform_matrix)
{
	mesh* plane_mesh = asset_manager_get_mesh("plane");
	bgfx_set_vertex_buffer(0, plane_mesh->vb_handle, 0, plane_mesh->vertex_count);
	bgfx_set_index_buffer(plane_mesh->idb_handle, 0, plane_mesh->index_count);

	bgfx_set_texture(0, graphics_texture_sampler, image->handle, BGFX_SAMPLER_POINT);

	bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA | BGFX_STATE_MSAA, 0);

	bgfx_set_transform(&transform_matrix, 1);

	bgfx_submit(0, asset_manager_get_shader("gui")->handle, 0, false);
}

mat4 graphics_create_model_matrix(vec3 pos, float rot, vec3 rot_axis, vec3 scale)
{
	mat4 matrix = mat4(1.0f);
	matrix *= translate(matrix, pos);
	matrix *= rotate(matrix, rot, rot_axis);
	matrix *= glm::scale(matrix, scale * 4.0f);
	return matrix;
}