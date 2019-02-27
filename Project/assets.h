#pragma once

#include "mesh.h"
#include "image.h"
#include "shader.h"
#include "font.h"

// meshes
extern mesh cube_mesh;
extern mesh robot_mesh;
extern mesh terrain_mesh;

// images
extern image shoot_image;
extern image test_image;
extern image healthbox_image;
extern image fhealthbar_image;
extern image ehealthbar_image;
extern image combat_log_bg_image;
extern image action_bar_bg_image;
extern image action_bar_top_bg_image;
extern image action_image;
extern image action_hover_image;
extern image mode_text_move_image;
extern image mode_text_throw_image;
extern image mode_text_shooting_image;
extern image selected_entity_image;

extern image action_move_image;
extern image action_shoot_image;
extern image action_throw_image;

// shaders
extern shader_program default_shader;
extern shader_program diffuse_shader;
extern shader_program gui_shader;

// fonts 
extern font* inconsolata_font;

void assets_init();