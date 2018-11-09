#include "graphics.h"

#include <stdio.h>

#include <vector>

#include "shader.h"
#include "model.h"

/* ------- Globals --------- */
bgfx_renderer_type_t renderer_type;
bool graphics_debug_draw_normals = true;

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

void graphics_init(int window_width, int window_height, float fov)
{
	bgfx_init_t init;
	bgfx_init_ctor(&init);
	init.type = BGFX_RENDERER_TYPE_OPENGL;

	bgfx_init(&init);

	bgfx_reset(window_width, window_height, BGFX_RESET_VSYNC, init.resolution.format);

	bgfx_set_debug(BGFX_DEBUG_TEXT | BGFX_DEBUG_WIREFRAME);

	renderer_type = bgfx_get_renderer_type();

	bgfx_set_view_clear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

	// setup view and projection matrix
	mat4 view_matrix = lookAt(vec3(0.0f, 10.0f, -15.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 projection_matrix = perspective(radians(fov), (float) window_width / (float) window_height, 0.1f, 10000.0f);

	bgfx_set_view_transform(0, &view_matrix, &projection_matrix);

	bgfx_set_view_rect(0, 0, 0, window_width, window_height);
}

void draw_model(model model, mat4 model_matrix)
{
	bgfx_set_vertex_buffer(0, model.vb_handle, 0, model.vertex_count);
	bgfx_set_index_buffer(model.vidb_handle, 0, model.vertex_index_count);

	bgfx_set_state(BGFX_STATE_WRITE_R | BGFX_STATE_WRITE_G | BGFX_STATE_WRITE_B | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW, 0);

	bgfx_set_transform(&model_matrix, 1);

	bgfx_submit(0, diffuse_shader.handle, 0, false);

	// This is very slow, but it's only for debugging so!?!
	if(graphics_debug_draw_normals)
	{
		bgfx_vertex_decl_t decl;
		bgfx_vertex_decl_begin(&decl, BGFX_RENDERER_TYPE_NOOP);
		bgfx_vertex_decl_add(&decl, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
		bgfx_vertex_decl_end(&decl);

		std::vector<vec3> vertices;

		for (u32 i = 0; i < model.vertex_index_count; i++)
		{
			pos_normal_vertex vertex = model.vertices[model.indices[i]];
			u8* norm = (u8*) &vertex.normal;
			i16 x_norm = norm[0] - 128;
			i16 y_norm = norm[1] - 128;
			i16 z_norm = 128;

			vertices.push_back(vec3(vertex.x - 0.025f, vertex.y + (y_norm / 255.0f) + 0.025f, vertex.z));
			vertices.push_back(vec3(vertex.x - 0.025f, vertex.y, vertex.z + (z_norm / 256.0f)));
			vertices.push_back(vec3(vertex.x + 0.025f + (x_norm / 255.0f), vertex.y, vertex.z + (z_norm / 256.0f)));
			vertices.push_back(vec3(vertex.x + 0.025f + (x_norm / 255.0f), vertex.y + (y_norm / 255.0f), vertex.z));
			vertices.push_back(vec3(vertex.x - 0.025f, vertex.y + (y_norm / 255.0f), vertex.z ));
			vertices.push_back(vec3(vertex.x + 0.025f + (x_norm / 255.0f), vertex.y, vertex.z + (z_norm / 256.0f)));
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

		bgfx_set_transform(&model_matrix, 1);

		bgfx_set_state(BGFX_STATE_WRITE_R, 0);
		// @Note: we just use 0 for the default red shader
		//bgfx_submit(0, default_shader.handle, 0, false);

		bgfx_destroy_vertex_buffer(vb_handle);
		bgfx_destroy_index_buffer(id_handle);
	}
}