// @Todo: add x11, wayland, mir support for windows?
#ifdef _WIN32
	#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#ifdef __APPLE__
	#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <stdio.h>
#include <assert.h>

#include <vector>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <bgfx/c99/bgfx.h>
#include <bgfx/c99/platform.h>

using namespace glm;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define FOV 6.0f

struct model
{
	bgfx_vertex_buffer_handle_t vb_handle;
	u32 vertex_count;

	bgfx_index_buffer_handle_t id_handle;
	u32 index_count;
};

struct shader_program
{
	bgfx_program_handle_t handle;
};

/* ------- Globals --------- */
bgfx_renderer_type_t renderer_type;

model cube_model;
shader_program default_shader_program;

void load_models();
void load_shaders();
void draw(float dt);

// Load an individual vertex or fragment shader from a file path
bgfx_shader_handle_t load_shader(char* filename)
{
	FILE* file = fopen(filename, "r");

	if(file == NULL)
	{
		printf("Failed to load shader from file %s\n", filename);

		// @Safety
		return {};
	}

	// @Cleanup: if we do a lot of file io we could make a wrapper for this stuff?
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	void* shader_data = malloc(file_size);
	fread(shader_data, file_size, 1, file);

	bgfx_shader_handle_t shader = bgfx_create_shader(bgfx_make_ref(file, file_size));

	// We're done with the data and file now
	fclose(file);
	free(shader_data);

	// @Todo: bgfx_set_shader_name(shader, filename, strlen(filename));
	return shader;
}

// @Todo: (for this and load model) use names instead of paths? i.e. shader vs_cube resolves to res/shaders/dx11/vs_cube.bin and model cube resolves to res/models/cube.obj
// @Todo: add support for other renderers with different shaders for each type? it's not hard, just different folders for each shader type, compile with shaderc, donezo
shader_program load_shader_program(char* vs_filename, char* fs_filename)
{
	if(renderer_type != BGFX_RENDERER_TYPE_OPENGL)
	{
		printf("ogl is currently the only supported renderer :(\n");
		// @Safety: we should always return SOMETHING, not something blank! (default shader program?)
		return {};
	}

	bgfx_shader_handle_t vs_handle = load_shader(vs_filename);
	bgfx_shader_handle_t fs_handle = load_shader(fs_filename);

	shader_program shader;
	shader.handle = bgfx_create_program(vs_handle, fs_handle, true);
	return shader;
}

// @Cleanup
// @Todo: I think this is a mesh, not a model!
// @Todo: Remove use of std::?
model load_obj_model(char* filename)
{
	FILE* file = fopen(filename, "r");

	if(file == NULL)
	{
		printf("Failed to load obj model from file %s\n", filename);

		// @Safety: safe return a default model? like a cube?
		return{};
	}

	std::vector<unsigned int> vertex_ids, uv_ids, normal_ids;
	std::vector<vec3> temp_vertices;
	std::vector<vec2> temp_uvs;
	std::vector<vec3> temp_normals;

	while(true)
	{
		// @Todo: the first word of the line can be longer than 128 apparently?
		char header[128];
		int res = fscanf(file, "%s", header);
		if(res == EOF) break;

		if(strcmp(header, "v") == 0)
		{
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if(strcmp(header, "vt") == 0)
		{
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if(strcmp(header, "vn") == 0)
		{
			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if(strcmp(header, "f") == 0)
		{
			u32 vertex_index[3], uv_index[3], normal_index[3];
			fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertex_index[0], &uv_index[0], &normal_index[0], &vertex_index[1], &uv_index[1], &normal_index[1], &vertex_index[2], &uv_index[2], &normal_index[2]);
			vertex_ids.push_back(vertex_index[0]);
			vertex_ids.push_back(vertex_index[1]);
			vertex_ids.push_back(vertex_index[2]);
			uv_ids.push_back(uv_index[0]);
			uv_ids.push_back(uv_index[1]);
			uv_ids.push_back(uv_index[2]);
			normal_ids.push_back(normal_index[0]);
			normal_ids.push_back(normal_index[1]);
			normal_ids.push_back(normal_index[2]);
		}
	}

	fclose(file);

	bgfx_vertex_decl_t decl;
	bgfx_vertex_decl_begin(&decl, BGFX_RENDERER_TYPE_NOOP);
	bgfx_vertex_decl_add(&decl, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT, false, false);
	bgfx_vertex_decl_add(&decl, BGFX_ATTRIB_NORMAL, 3, BGFX_ATTRIB_TYPE_FLOAT, true, false);
	bgfx_vertex_decl_end(&decl);

	/*static vec3 vertices[] =
	{
		{ 1.000000f, -1.000000f, -1.000000f },
		{ 1.000000f, -1.000000f, 1.000000f },
		{ -1.000000f, -1.000000f, 1.000000f },
		{ -1.000000f, -1.000000f, -1.000000f },
		{ 1.000000f, 1.000000f, -1.000000f },
		{ 0.999999f, 1.000000f, 1.000001f },
		{ -1.000000f, 1.000000f, 1.000000f },
		{ -1.000000f, 1.000000f, -1.000000f },
	};*/

	/*static const u16 indices[] =
	{
		4, 0, 3, 4, 3, 7, 2, 6, 7, 2, 7, 3, 1, 5, 2, 5, 6, 2, 0, 4, 1, 4, 5, 1, 4, 7, 5, 7, 6, 5, 0, 1, 2, 0, 2, 3,
	};*/

	/*model result = {};
	result.vb_handle = bgfx_create_vertex_buffer(bgfx_make_ref(vertices, sizeof(vertices)), &decl, BGFX_BUFFER_NONE);
	result.vertex_count = sizeof(vertices);
	
	result.id_handle = bgfx_create_index_buffer(bgfx_make_ref(indices, sizeof(indices)), BGFX_BUFFER_NONE);
	result.index_count = sizeof(indices);*/

	model result = {};

	result.vertex_count = temp_vertices.size();
	u32 vertex_byte_count = result.vertex_count * sizeof(vec3);
	vec3* vertices = (vec3*) malloc(vertex_byte_count);

	for (int i = 0; i < result.vertex_count; i++)
	{
		vertices[i] = temp_vertices[i];
	}

	result.vb_handle = bgfx_create_vertex_buffer(bgfx_make_ref(vertices, vertex_byte_count), &decl, BGFX_BUFFER_NONE);

	result.index_count = vertex_ids.size();
	u32 index_byte_count = result.index_count * sizeof(u16);
	u16* indices = (u16*) malloc(index_byte_count);

	for (int i = 0; i < result.index_count; i++)
	{
		indices[i] = vertex_ids[i] - 1;
	}

	result.id_handle = bgfx_create_index_buffer(bgfx_make_ref(indices, index_byte_count), BGFX_BUFFER_NONE);

	// @Todo: free dynamic memory?
	return result;
}

int main()
{
	// Setup GLFW
	GLFWwindow* window;

	assert(glfwInit());

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Game Window", NULL, NULL);

	assert(window);

	glfwMakeContextCurrent(window);

	// Setup BGFX
	u32 debug = BGFX_DEBUG_TEXT;
	u32 reset = BGFX_RESET_VSYNC;

	bgfx_platform_data_t platform_data = {};

	// @Todo: add x11, wayland, mir support for windows?
	#ifdef _WIN32
		platform_data.nwh = glfwGetWin32Window(window);
	#endif

	#ifdef __APPLE__
		platformData.nwh = glfwGetCocoaWindow(mWindow);
	#endif

	bgfx_set_platform_data(&platform_data);

	bgfx_init_t init;
	bgfx_init_ctor(&init);
	init.type = BGFX_RENDERER_TYPE_OPENGL;

	bgfx_init(&init);

	renderer_type = bgfx_get_renderer_type();

	bgfx_reset(WINDOW_WIDTH, WINDOW_HEIGHT, reset, init.resolution.format);

	bgfx_set_debug(debug);

	bgfx_set_view_clear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

	float lastTime = 0;
	float time, dt;

	load_models();
	load_shaders();

	// setup view and projection matrix
	mat4 view_matrix = lookAt(vec3(0.0f, 20.0f, -15.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 projection_matrix = perspective(radians(FOV), (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT, 0.1f, 10000.0f);

	bgfx_set_view_transform(0, &view_matrix, &projection_matrix);

	bgfx_set_view_rect(0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	while(!glfwWindowShouldClose(window))
	{
		time = (float) glfwGetTime();
		dt = time - lastTime;
		lastTime = time;

		glfwPollEvents();

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx_touch(0);

		bgfx_dbg_text_clear(0, false);

		bgfx_dbg_text_printf(0, 0, 0x0f, "Last frame time: %.2fms, FPS: %.0f", dt * 1000.0f, 1 / dt);

		draw(dt);

		bgfx_frame(false);
	}

	bgfx_shutdown();
	glfwTerminate();

	return 0;
}

void load_models()
{
	cube_model = load_obj_model("res/untitled.obj");
}

void load_shaders()
{
	default_shader_program = load_shader_program("res/vs_cubes.bin", "res/fs_cubes.bin");
}

void draw_model(model model)
{
	bgfx_set_vertex_buffer(0, model.vb_handle, 0, model.vertex_count);
	bgfx_set_index_buffer(model.id_handle, 0, model.index_count);

	bgfx_set_state(BGFX_STATE_WRITE_R | BGFX_STATE_WRITE_G | BGFX_STATE_WRITE_B | BGFX_STATE_WRITE_A, 0);

	bgfx_submit(0, default_shader_program.handle, 0, false);
}

float sinceStart = 0;

void draw(float dt)
{
	sinceStart += dt;
	mat4 model_matrix = mat4(1.0f);
	model_matrix = rotate(model_matrix, radians(sinceStart * 70), vec3(0.0f, 1.0f, 0.0f));
	bgfx_set_transform(&model_matrix, 1);
	draw_model(cube_model);
}

