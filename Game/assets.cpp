#include "assets.h"

#include "map.h"

// meshes
mesh cube_mesh;
mesh robot_mesh;
mesh terrain_mesh;

// images
image shoot_image;
image test_image;
image healthbox_image;
image action_bar_bg_image;

// shaders
shader_program default_shader;
shader_program diffuse_shader;
shader_program gui_shader;

void assets_init()
{
	// meshes
	cube_mesh = load_obj_mesh("res/mesh/cube.obj");
	robot_mesh = load_obj_mesh("res/mesh/robot.obj");
	terrain_mesh = load_terrain_mesh();

	// images
	shoot_image = load_image("res/horse.png");
	test_image = load_image("res/test.png");
	healthbox_image = load_image("res/healthbox.png");
	action_bar_bg_image = load_image("res/action_bar_bg.png");

	// shaders
	default_shader = load_shader_program("res/shader/default_vs.bin", "res/shader/default_fs.bin");
	diffuse_shader = load_shader_program("res/shader/vs_cubes.bin", "res/shader/fs_cubes.bin");
	gui_shader = load_shader_program("res/shader/gui_vs.bin", "res/shader/gui_fs.bin");
}