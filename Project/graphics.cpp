#include "graphics.h"

#include <stdio.h>

#include <vector>

#include "asset_manager.h"
#include "mesh.h"

/* ------- Globals --------- */
bgfx_renderer_type_t graphics_renderer_type;
bool graphics_debug_draw_normals = false;

mat4 graphics_view_matrix;
mat4 graphics_projection_matrix;

u32 graphics_projection_width;
u32 graphics_projection_height; 

bgfx_uniform_handle_t texture_sampler;

// @Todo: cleanup!!!
void bgfx_fatal_callback(bgfx_callback_interface_t* _this, const char* _filePath, u16 _line, bgfx_fatal_t _code, const char* _str)
{
	printf("FATAL ERROR!!!");
}

extern void bgfx_log_callback(bgfx_callback_interface_t* _this, const char* _filePath, u16 _line, const char* _format, va_list _argList)
{
	char temp[8192];
	char* out = temp;
	i32 len = vsnprintf(out, sizeof(temp), _format, _argList);
	if ((i32)sizeof(temp) < len)
	{
		out = (char*)alloca(len + 1);
		len = vsnprintf(out, len, _format, _argList);
	}
	out[len] = '\0';
	printf(out);
}

u32 bgfx_cache_read_size_callback(bgfx_callback_interface_t* _this, u64 _id) { return 0; }
bool bgfx_cache_read_callback(bgfx_callback_interface_t* _this, u64 _id, void* _data, u32 _size) { return 0; }
void bgfx_cache_write_callback(bgfx_callback_interface_t* _this, u64 _id, const void* _data, u32 _size) {}

void graphics_init(int window_width, int window_height)
{
	bgfx_init_t init;
	bgfx_init_ctor(&init);
	init.type = BGFX_RENDERER_TYPE_OPENGL;

	bgfx_init(&init);

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

	texture_sampler = bgfx_create_uniform("textureSampler", BGFX_UNIFORM_TYPE_SAMPLER, 1);
}

void graphics_draw_mesh(mesh* mesh, mat4 transform_matrix)
{
	bgfx_set_vertex_buffer(0, mesh->vb_handle, 0, mesh->vertex_count);
	bgfx_set_index_buffer(mesh->idb_handle, 0, mesh->index_count);

	bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA, 0);

	bgfx_set_transform(&transform_matrix, 1);

	bgfx_submit(0, asset_manager_get_shader("diffuse")->handle, 0, false);

	// This is very slow, but it's only for debugging so!?!
	if(graphics_debug_draw_normals)
	{
		bgfx_vertex_decl_t decl;
		bgfx_vertex_decl_begin(&decl, BGFX_RENDERER_TYPE_NOOP);
		bgfx_vertex_decl_add(&decl, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
		bgfx_vertex_decl_end(&decl);

		std::vector<vec3> vertices;

		float draw_width = 0.05f;

		for (u32 i = 0; i < mesh->vertex_count; i++)
		{
			pos_normal_vertex vertex = mesh->vertices[i];
			u8* norm = (u8*) &vertex.normal;
			i16 x_norm = norm[0] - 128;
			i16 y_norm = norm[1] - 128;
			i16 z_norm = norm[2] - 128;

			float x_min = vertex.x;
			float y_min = vertex.y;
			float z_min = vertex.z;

			float x_max = vertex.x + (x_norm / 128.0f);
			float y_max = vertex.y + (y_norm / 128.0f);
			float z_max = vertex.z + (z_norm / 128.0f);

			vertices.push_back(vec3(x_min - draw_width, y_min, z_min));
			vertices.push_back(vec3(x_max, y_max, z_max));
			vertices.push_back(vec3(x_min + draw_width, y_min, z_min));
		}

		vec3* vertices_data = (vec3*) malloc(vertices.size() * sizeof(vec3));
		u16* indices_data = (u16*) malloc(vertices.size() * sizeof(u16));

		for (u32 i = 0; i < vertices.size(); i++)
		{
			vertices_data[i] = vertices[i];
			indices_data[i] = i;
		}

		const bgfx_memory_t* vertices_mem = bgfx_make_ref(vertices_data, vertices.size() * sizeof(vec3));
		bgfx_vertex_buffer_handle_t vb_handle = bgfx_create_vertex_buffer(vertices_mem, &decl, BGFX_BUFFER_NONE);
		bgfx_make_ref_release(vertices_data, vertices.size() * sizeof(vec3), 0, 0);

		const bgfx_memory_t* indices_mem = bgfx_make_ref(indices_data, vertices.size() * sizeof(u16));
		bgfx_index_buffer_handle_t id_handle = bgfx_create_index_buffer(indices_mem, BGFX_BUFFER_NONE);
		bgfx_make_ref_release(indices_data, vertices.size() * sizeof(u16), 0, 0);

		bgfx_set_vertex_buffer(0, vb_handle, 0, vertices.size());
		bgfx_set_index_buffer(id_handle, 0, vertices.size());

		bgfx_set_transform(&transform_matrix, 1);

		bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_MSAA, 0);

		bgfx_submit(0, asset_manager_get_shader("gui")->handle, 0, false);

		bgfx_destroy_vertex_buffer(vb_handle);
		bgfx_destroy_index_buffer(id_handle);
	}
}

void graphics_draw_image(image* image, mat4 transform_matrix)
{
	mesh* plane_mesh = asset_manager_get_mesh("plane");
	bgfx_set_vertex_buffer(0, plane_mesh->vb_handle, 0, plane_mesh->vertex_count);
	bgfx_set_index_buffer(plane_mesh->idb_handle, 0, plane_mesh->index_count);

	bgfx_set_texture(0, texture_sampler, image->handle, BGFX_SAMPLER_POINT);

	bgfx_set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_BLEND_ALPHA, 0);

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