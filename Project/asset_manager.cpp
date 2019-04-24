#include "asset_manager.h"

#include "asset.h"
#include "map.h"
#include "map_gen.h"
#include "hashtable.h"

#include <vector>

hashtable* assets = hashtable_create(32, sizeof(asset*));

void asset_manager_init()
{
	// meshes
	asset_manager_register(load_obj_mesh("plane", "res/mesh/plane.obj"));
	asset_manager_register(load_obj_mesh("robot", "res/mesh/robot.obj"));
	asset_manager_register(load_obj_mesh("cube", "res/mesh/cube.obj"));
	asset_manager_register(load_obj_mesh("light", "res/mesh/light.obj"));

	// images
	asset_manager_register(load_image("res/img/action_button.png"));
	asset_manager_register(load_image("res/img/action_button_hover.png"));
	asset_manager_register(load_image("res/img/action_end.png"));
	asset_manager_register(load_image("res/img/action_move.png"));
	asset_manager_register(load_image("res/img/action_shoot.png"));
	asset_manager_register(load_image("res/img/action_throw.png"));
	asset_manager_register(load_image("res/img/actionbar_bg.png"));
	asset_manager_register(load_image("res/img/actionbar_top_bg.png"));
	asset_manager_register(load_image("res/img/combat_log_bg.png"));
	asset_manager_register(load_image("res/img/enemy_healthbar.png"));
	asset_manager_register(load_image("res/img/friendly_healthbar.png"));
	asset_manager_register(load_image("res/img/healthbox.png"));
	asset_manager_register(load_image("res/img/selected_entity.png"));

	// shaders
	asset_manager_register(load_shader_program("diffuse", "res/shader/diffuse.vs", "res/shader/diffuse.fs"));
	asset_manager_register(load_shader_program("colored", "res/shader/colored.vs", "res/shader/colored.fs"));
	asset_manager_register(load_shader_program("gui", "res/shader/gui.vs", "res/shader/gui.fs"));
	asset_manager_register(load_shader_program("font", "res/shader/font.vs", "res/shader/font.fs"));

	// fonts
	asset_manager_register(load_font("inconsolata", "res/font/inconsolata.ttf"));

	asset_manager_register(map_gen_terrain_mesh());
}

void asset_manager_register(asset* as)
{
	debug_assert(as, "Tried to register null asset");

	hashtable_put(assets, hashtable_hash_str(as->asset_id), &as);
}

void asset_manager_register(mesh* as)
{
	asset_manager_register((asset*) as);
}

void asset_manager_register(font* as)
{
	asset_manager_register((asset*) as);
}

void asset_manager_register(shader_program* as)
{
	asset_manager_register((asset*) as);
}

void asset_manager_register(image* as)
{
	asset_manager_register((asset*) as);
}

asset* asset_manager_get_asset(char* id)
{
	debug_assert(id, "Must provide an asset id");

	asset* as = *((asset**) hashtable_get(assets, hashtable_hash_str(id)));

	debug_assert(as, "Unable to find asset");

	return as;
}

asset* asset_manager_get_asset(char* id, asset_type type)
{
	asset* as = asset_manager_get_asset(id);

	debug_assert(type == as->asset_type, "Asset of different type than expected");

	return as;
}

mesh* asset_manager_get_mesh(char* id)
{
	return (mesh*) asset_manager_get_asset(id, ASSET_TYPE_MESH);
}

font* asset_manager_get_font(char* id)
{
	return (font*) asset_manager_get_asset(id, ASSET_TYPE_FONT);
}

shader_program* asset_manager_get_shader(char* id)
{
	return (shader_program*) asset_manager_get_asset(id, ASSET_TYPE_SHADER);
}

image* asset_manager_get_image(char* id)
{
	return (image*) asset_manager_get_asset(id, ASSET_TYPE_IMAGE);
}