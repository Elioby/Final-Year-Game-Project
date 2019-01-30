#include "gui.h"

#include <glm/glm.hpp>

#include <stdio.h>

#include "shader.h"
#include "input.h"

struct gui_mesh_vertex
{
	float x, y;
	float texX, texY;
};

// @Cleanup
bgfx_vertex_buffer_handle_t gui_mesh_vb_handle;
bgfx_index_buffer_handle_t gui_mesh_idb_handle;
int vc;
int ic;

bgfx_uniform_handle_t textureSampler;

void gui_init() 
{
	bgfx_set_view_rect(1, 0, 0, graphics_projection_width, graphics_projection_height);

	textureSampler = bgfx_create_uniform("textureSampler", BGFX_UNIFORM_TYPE_SAMPLER, 1);

	bgfx_vertex_decl_t decl;
	bgfx_vertex_decl_begin(&decl, BGFX_RENDERER_TYPE_NOOP);
	bgfx_vertex_decl_add(&decl, BGFX_ATTRIB_POSITION, 2, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_add(&decl, BGFX_ATTRIB_TEXCOORD0, 2, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_end(&decl);

	u32 vertex_count = 4;
	u32 vertex_byte_count = vertex_count * sizeof(gui_mesh_vertex);
	gui_mesh_vertex* verts = (gui_mesh_vertex*) malloc(vertex_byte_count);

	verts[0] = { -1.0f, -1.0f, 0.0f, 1.0f };
	verts[1] = { 1.0f, -1.0, 1.0f, 1.0f };
	verts[2] = { -1.0f, 1.0f, 0.0f, 0.0f };
	verts[3] = { 1.0f, 1.0f, 1.0f, 0.0f };

	const bgfx_memory_t* vertices_mem = bgfx_make_ref(verts, vertex_byte_count);
	gui_mesh_vb_handle = bgfx_create_vertex_buffer(vertices_mem, &decl, BGFX_BUFFER_NONE);
	bgfx_make_ref_release(vertices_mem, vertex_byte_count, 0, 0);

	u32 index_count = 6;
	u32 index_byte_count = index_count * sizeof(u16);
	u16* inds = (u16*) malloc(index_byte_count);

	inds[0] = 1;
	inds[1] = 2;
	inds[2] = 0;
	inds[3] = 1;
	inds[5] = 3;
	inds[4] = 2;

	const bgfx_memory_t* vertex_index_mem = bgfx_make_ref(inds, index_byte_count);
	gui_mesh_idb_handle = bgfx_create_index_buffer(vertex_index_mem, BGFX_BUFFER_NONE);
	bgfx_make_ref_release(vertex_index_mem, index_byte_count, 0, 0);

	vc = vertex_count;
	ic = index_count;
}

void gui_draw_image(image image, vec2 pos, vec2 rot, vec2 scale)
{
	mat4 transform_matrix = mat4(1.0f);
	transform_matrix *= translate(transform_matrix, vec3(pos, 0.0f));
	transform_matrix *= rotate(transform_matrix, 0.0f, vec3(rot, 1.0f));
	transform_matrix *= glm::scale(transform_matrix, vec3(scale, 1.0f));
	
	gui_draw_image(image, transform_matrix);
}

void gui_draw_image(image image, mat4 transform_matrix)
{
	bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_MSAA | BGFX_STATE_BLEND_ALPHA, 0);

	bgfx_set_vertex_buffer(0, gui_mesh_vb_handle, 0, vc);
	bgfx_set_index_buffer(gui_mesh_idb_handle, 0, ic);

	bgfx_set_texture(0, textureSampler, image.handle, 0);

	bgfx_set_transform(&transform_matrix, 1);

	bgfx_submit(1, gui_shader.handle, 0, false);
}

void gui_draw_button(gui_button button)
{
	vec2 pos = vec2((button.x + button.width / 2) / (graphics_projection_width * 2.0f) - 0.25f, 
		(button.y + button.height / 2) / (graphics_projection_height * 2.0f) - 0.25f);

	vec2 scale = vec2(button.width / (float) graphics_projection_width, button.height / (float) graphics_projection_height);

	image img;
	if(button.has_hover && input_mouse_x >= button.x && input_mouse_x <= button.x + button.width && input_mouse_y >= button.y && input_mouse_y <= button.y + button.height)
	{
		img = button.hover_img;

		if(input_mouse_button_left == INPUT_MOUSE_BUTTON_DOWN_START) 
		{
			if(button.click_callback != NULL) button.click_callback();
		}
	}
	else
	{
		img = button.img;
	}

	gui_draw_image(img, pos, vec2(1.0f), scale);
}