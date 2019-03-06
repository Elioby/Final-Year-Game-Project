#include "asset_manager.h"
#include "asset.h"

#include <vector>

std::vector<asset*> assets;

void asset_manager_init()
{
	asset_manager_register(load_obj_mesh("plane", "res/mesh/plane.obj"));
	asset_manager_register(load_obj_mesh("robot", "res/mesh/robot.obj"));
	asset_manager_register(load_obj_mesh("cube", "res/mesh/cube.obj"));

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
	asset_manager_register(load_shader_program("diffuse", "res/shader/diffuse_vs.bin", "res/shader/diffuse_fs.bin"));
	asset_manager_register(load_shader_program("gui", "res/shader/gui_vs.bin", "res/shader/gui_fs.bin"));

	// fonts
	asset_manager_register(load_font("inconsolata", "res/font/inconsolata.ttf"));
}

void asset_manager_register(asset* as)
{
	assert(as && "Tried to register null asset");

	assets.push_back(as);
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

asset* asset_manager_get_asset(char* id, asset_type type)
{
	assert(id && "Must provide an asset id");

	for(u32 i = 0; i < assets.size(); i++)
	{
		asset* as = assets[i];
		if(as->asset_id && strcmp(as->asset_id, id) == 0)
		{
			if(as->asset_type != type)
			{
				printf("Asset with id %s was not of expected type %i\n", id, type);
				assert(false && "asset_get type not the same as asset type at id");
			}

			return as;
		}
	}

	printf("Unable to find asset with id %s\n", id);
	assert(false && "Unable to find asset");
	return NULL;
}

mesh* asset_manager_get_mesh(char* id)
{
	return (mesh*)asset_manager_get_asset(id, ASSET_TYPE_MESH);
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