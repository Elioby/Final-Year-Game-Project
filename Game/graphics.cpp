#include "graphics.h"

#include <stdio.h>

#include "shader.h"
#include "model.h"

/* ------- Globals --------- */
bgfx_renderer_type_t renderer_type;
bool graphics_debug_draw_normals = false;

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

	bgfx_set_debug(BGFX_DEBUG_TEXT);

	renderer_type = bgfx_get_renderer_type();

	bgfx_set_view_clear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

	// setup view and projection matrix
	mat4 view_matrix = lookAt(vec3(0.0f, 10.0f, -15.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 projection_matrix = perspective(radians(fov), (float) window_width / (float) window_height, 0.1f, 10000.0f);

	bgfx_set_view_transform(0, &view_matrix, &projection_matrix);

	bgfx_set_view_rect(0, 0, 0, window_width, window_height);
}

void draw_model(model model)
{
	bgfx_set_vertex_buffer(0, model.vb_handle, 0, model.vertex_count);
	bgfx_set_index_buffer(model.vidb_handle, 0, model.vertex_index_count);

	bgfx_set_state(BGFX_STATE_WRITE_R | BGFX_STATE_WRITE_G | BGFX_STATE_WRITE_B | BGFX_STATE_WRITE_A | BGFX_STATE_CULL_CW, 0);

	bgfx_submit(0, diffuse_shader.handle, 0, false);

	// This is very slow, but it's only for debugging so!?!
	if(graphics_debug_draw_normals)
	{

	}
}