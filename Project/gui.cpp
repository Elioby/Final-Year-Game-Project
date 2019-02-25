#include "gui.h"

#include <glm/glm.hpp>

#include <vector>
#include <stdio.h>

#include "assets.h"
#include "input.h"
#include "graphics.h"

std::vector<button*> buttons;

void gui_init() 
{
	bgfx_set_view_rect(1, 0, 0, graphics_projection_width, graphics_projection_height);
}

// returns true if the gui handled the mouse click
bool gui_update()
{
	bool handled = false;

	for(u32 i = 0; i < buttons.size(); i++)
	{
		button button = *buttons[i];
		if(input_mouse_button_left == INPUT_MOUSE_BUTTON_UP_START && input_mouse_x >= button.x && input_mouse_x <= button.x + button.width 
			&& input_mouse_y >= button.y && input_mouse_y <= button.y + button.height)
		{
			if(button.click_callback) button.click_callback();
			handled = true;
		}
	}

	return handled;
}

button* gui_create_button()
{
	// @Todo: use of malloc :(
	button* butt = (button*) malloc(sizeof(button));
	buttons.push_back(butt);

	return butt;
}

void gui_draw_image(image image, u32 x, u32 y, u32 width, u32 height)
{
	vec2 pos = vec2(x / (graphics_projection_width * 2.0f) - 0.25f,
		y / (graphics_projection_height * 2.0f) - 0.25f);

	vec2 scale = vec2(width / (float) graphics_projection_width, height / (float) graphics_projection_height);

	mat4 transform_matrix = mat4(1.0f);
	transform_matrix *= translate(transform_matrix, vec3(pos, 0.0f));
	transform_matrix *= rotate(transform_matrix, 0.0f, vec3(1.0f));
	transform_matrix *= glm::scale(transform_matrix, vec3(scale, 1.0f));
	
	gui_draw_image(image, transform_matrix);
}

void gui_draw_image(image image, mat4 transform_matrix)
{
	bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_MSAA | BGFX_STATE_BLEND_ALPHA, 0);

	bgfx_set_vertex_buffer(0, plane_mesh.vb_handle, 0, plane_mesh.vertex_count);
	bgfx_set_index_buffer(plane_mesh.idb_handle, 0, plane_mesh.index_count);

	bgfx_set_texture(0, texture_sampler, image.handle, 0);

	bgfx_set_transform(&transform_matrix, 1);

	bgfx_submit(1, gui_shader.handle, 0, false);
}

void gui_draw_button(button button)
{
	image img;
	if(input_mouse_x >= button.x && input_mouse_x <= button.x + button.width && input_mouse_y >= button.y && input_mouse_y <= button.y + button.height)
	{
		img = button.hover_bg_img;
	}
	else
	{
		img = button.bg_img;
	}

	gui_draw_image(img, button.x, button.y, button.width, button.height);
	gui_draw_image(button.icon_img, button.x + button.width / 8, button.y + button.height / 8, button.width - button.width / 4, button.height - button.height / 4);
}

// for text
u16 inds[] = { 1, 2, 0, 1, 3, 2 };

void gui_draw_text(dynstr* text, font* font, u32 x, u32 y, float scale)
{
	u32 i = 0;

	scale *= 2.0f;

	float x1 = 0;
	float y1 = 0;

	char* all_verts = (char*) malloc(sizeof(pos_normal_vertex) * 4 * (text->len + 1));

	while(true)
	{
		char c = text->raw[i++];

		if(c == 0) break;

		const stbtt_bakedchar *b = font->char_data + c - 32;
		stbtt_aligned_quad q = {};
		stbtt_GetBakedQuad(font->char_data, font->width, font->height, c - 32, &x1, &y1, &q, 1);// @Volatile: 1=opengl & d3d10+,0=d3d9

		pos_normal_vertex* verts = (pos_normal_vertex*) (all_verts + i * sizeof(pos_normal_vertex) * 4);

		verts[0] = { (q.x0 * scale) / (float) graphics_projection_width - 1.0f, (q.y0 * scale) / (float) graphics_projection_height - 1.0f, 0.0f, q.s0, q.t1, 0 };
		verts[1] = { (q.x1 * scale) / (float) graphics_projection_width - 1.0f, (q.y0 * scale) / (float) graphics_projection_height - 1.0f, 0.0f, q.s1, q.t1, 0 };
		verts[2] = { (q.x0 * scale) / (float) graphics_projection_width - 1.0f, (q.y1 * scale) / (float) graphics_projection_height - 1.0f, 0.0f, q.s0, q.t0, 0 };
		verts[3] = { (q.x1 * scale) / (float) graphics_projection_width - 1.0f, (q.y1 * scale) / (float) graphics_projection_height - 1.0f, 0.0f, q.s1, q.t0, 0 };

		mesh m = mesh_create(verts, 4, inds, 6);

		bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_MSAA | BGFX_STATE_BLEND_ALPHA, 0);

		bgfx_set_vertex_buffer(0, m.vb_handle, 0, m.vertex_count);
		bgfx_set_index_buffer(m.idb_handle, 0, m.index_count);

		bgfx_set_texture(0, texture_sampler, font->img.handle, 0);

		mat4 transform_matrix = mat4(1.0f); 
		transform_matrix *= translate(transform_matrix, vec3(((x1 / 32.0f) + (x / 2.0f)) / (float) graphics_projection_width,
			((((q.y0 - q.y1) * scale / 2.0f) - b->yoff * scale) / 2.0f + y / 2.0f) / (float) graphics_projection_height, 0.0f));
		transform_matrix *= rotate(transform_matrix, 0.0f, vec3(1.0f));
		transform_matrix *= glm::scale(transform_matrix, vec3(1.0f, 1.0f, 1.0f));

		bgfx_set_transform(&transform_matrix, 1);

		bgfx_submit(1, gui_shader.handle, 0, false);
		mesh_destroy(m);
	}
}