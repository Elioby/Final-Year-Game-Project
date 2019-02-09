#pragma once

#include "mesh.h"
#include "image.h"
#include "shader.h"

// meshes
extern mesh cube_mesh;
extern mesh robot_mesh;
extern mesh terrain_mesh;

// images
extern image shoot_image;
extern image test_image;
extern image healthbox_image;
extern image healthbar_image;
extern image action_bar_bg_image;
extern image action_bar_top_bg_image;
extern image action_image;
extern image action_hover_image;
extern image mode_text_move_image;
extern image mode_text_shooting_image;

extern image action_move_image;
extern image action_shoot_image;
extern image action_throw_image;

// shaders
extern shader_program default_shader;
extern shader_program diffuse_shader;
extern shader_program gui_shader;

void assets_init();