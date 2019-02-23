#include "assets.h"

#include "map.h"

// @Todo: cleanup!!!! change to just load all files in folder, and reference by string?

// meshes
mesh cube_mesh;
mesh robot_mesh;
mesh terrain_mesh;

// images
image shoot_image;
image test_image;
image healthbox_image;
image healthbar_image;
image action_bar_bg_image;
image action_bar_top_bg_image;
image action_image;
image action_hover_image;
image mode_text_shooting_image;
image mode_text_move_image;
image mode_text_throw_image;

image action_move_image;
image action_shoot_image;
image action_throw_image;

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
	healthbar_image = load_image("res/healthbar.png");
	action_bar_bg_image = load_image("res/action_bar_bg.png");
	action_bar_top_bg_image = load_image("res/action_bar_top_bg.png");
	action_image = load_image("res/action.png");
	action_hover_image = load_image("res/action_hover.png");
	mode_text_move_image = load_image("res/mode_text_move.png");
	mode_text_shooting_image = load_image("res/mode_text_shooting.png");
	mode_text_throw_image = load_image("res/mode_text_throw.png");

	action_move_image = load_image("res/action_move.png");
	action_shoot_image = load_image("res/action_shoot.png");
	action_throw_image = load_image("res/action_throw.png");

	// shaders
	default_shader = load_shader_program("res/shader/default_vs.bin", "res/shader/default_fs.bin");
	diffuse_shader = load_shader_program("res/shader/vs_cubes.bin", "res/shader/fs_cubes.bin");
	gui_shader = load_shader_program("res/shader/gui_vs.bin", "res/shader/gui_fs.bin");
}