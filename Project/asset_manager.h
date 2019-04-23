#ifndef ASSET_H
#define ASSET_H

#include "image.h"
#include "shader.h"
#include "font.h"
#include "asset.h"

void asset_manager_init();
void asset_manager_register(mesh* as);
void asset_manager_register(font* as);
void asset_manager_register(shader_program* as);
void asset_manager_register(image* as);

asset* asset_manager_get_asset(char* id);
asset* asset_manager_get_asset(char* id, asset_type type);
mesh* asset_manager_get_mesh(char* id);
font* asset_manager_get_font(char* id);
shader_program* asset_manager_get_shader(char* id);
image* asset_manager_get_image(char* id);

#endif