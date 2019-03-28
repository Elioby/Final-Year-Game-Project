#include "actionbar.h"

#include "graphics.h"
#include "asset_manager.h"
#include "action.h"
#include "entity.h"
#include "map.h"

#define ENABLE_COMBAT_LOG true

struct actionbar_msg_show
{
	dynstr* msg;
	float show_seconds;
	float seconds_since_start;
};

u32 actionbar_width;

std::vector<button*> actionbar_buttons;

actionbar_msg_show actionbar_msg = { dynstr_new(20) };

dynstr* ap_text = dynstr_new(12);
dynstr* combat_log_text = dynstr_new(200);

void action_move_mode();
void action_shoot_mode();
void action_throw_mode();
void action_end_turn();

void actionbar_init()
{
	actionbar_width = (u32) (graphics_projection_width / 1.9f);

	u32 button_width = graphics_projection_width / 20;

	image* bg = asset_manager_get_image("action_button");
	image* hover = asset_manager_get_image("action_button_hover");

	button* move_button = gui_create_button();
	move_button->width = button_width;
	move_button->height = button_width;
	move_button->x = graphics_projection_width / 2 - (move_button->width * 4) / 2;
	move_button->y = move_button->height / 8;
	move_button->icon_img = asset_manager_get_image("action_move");
	move_button->bg_img = bg;
	move_button->hover_bg_img = hover;
	move_button->click_callback = action_move_mode;
	actionbar_buttons.push_back(move_button);

	button* shoot_button = gui_create_button();
	shoot_button->width = button_width;
	shoot_button->height = button_width;
	shoot_button->x = graphics_projection_width / 2 - shoot_button->width / 2;
	shoot_button->y = shoot_button->height / 8;
	shoot_button->icon_img = asset_manager_get_image("action_shoot");
	shoot_button->bg_img = bg;
	shoot_button->hover_bg_img = hover;
	shoot_button->click_callback = action_shoot_mode;
	actionbar_buttons.push_back(shoot_button);

	button* throw_button = gui_create_button();
	throw_button->width = button_width;
	throw_button->height = button_width;
	throw_button->x = graphics_projection_width / 2 + (throw_button->width * 2) / 2;
	throw_button->y = throw_button->height / 8;
	throw_button->icon_img = asset_manager_get_image("action_throw");
	throw_button->bg_img = bg;
	throw_button->hover_bg_img = hover;
	throw_button->click_callback = action_throw_mode;
	actionbar_buttons.push_back(throw_button);

	button* end_button = gui_create_button();
	end_button->width = button_width;
	end_button->height = button_width;
	end_button->y = throw_button->height / 8;
	end_button->icon_img = asset_manager_get_image("action_end");
	end_button->bg_img = bg;
	end_button->hover_bg_img = hover;
	end_button->click_callback = action_end_turn;
	actionbar_buttons.push_back(end_button);

	float padding = 0.5f;
	float button_width_padding = button_width * (1.0f + padding);

	u32 buttons_start = (u32) (graphics_projection_width / 2 - ((actionbar_buttons.size() * button_width) + ((actionbar_buttons.size() - 1) * button_width * padding)) / 2.0f);
	for(u32 i = 0; i < actionbar_buttons.size(); i++)
	{
		button* b = actionbar_buttons[i];

		b->x = (u32) (buttons_start + button_width_padding * i);
	}
}

void actionbar_update(float dt)
{
	actionbar_msg.seconds_since_start += dt;
}

std::vector<vec3> poses;

void actionbar_draw()
{
	for (u32 i = 0; i < poses.size(); i++)
	{
		graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(poses[i], 0.0f, vec3(1.0f), vec3(1.0f, 0.25f, 1.0f)));
	}

	font* inconsolata_font = asset_manager_get_font("inconsolata");

	if(selected_entity)
	{
		// draw top action bar 
		if (current_action_mode != ACTION_MODE_SELECT_UNITS)
		{
			char* text;

			if (current_action_mode == ACTION_MODE_SHOOT)
			{
				text = "Shoot Mode";
			}
			else if (current_action_mode == ACTION_MODE_THROW)
			{
				text = "Throw Mode";
			}
			else
			{
				text = "Move Mode";
			}

			// bar bg
			u32 bar_width = graphics_projection_width / 4;
			u32 bar_height = graphics_projection_height / 12;
			gui_draw_image(asset_manager_get_image("actionbar_top_bg"), graphics_projection_width / 2 - bar_width / 2, graphics_projection_height - bar_height, bar_width, bar_height);

			// bar text
			float scale = 0.5f;
			u32 text_width = (u32)font_get_text_width(inconsolata_font, text, scale);
			u32 text_height = (u32)(128.0f * scale);

			gui_draw_text(inconsolata_font, text, graphics_projection_width / 2 - text_width / 2, graphics_projection_height - text_height, scale);
		}

		// draw bottom action bar
		gui_draw_image(asset_manager_get_image("actionbar_bg"), (graphics_projection_width / 2) - (actionbar_width / 2), 0,
			actionbar_width, graphics_projection_width / 30);

		dynstr_clear(ap_text);

		dynstr_append(ap_text, "AP: %i / %i", selected_entity->ap, selected_entity->max_ap);
		gui_draw_text(inconsolata_font, ap_text, (graphics_projection_width / 2) - (actionbar_width / 2) + 15, 15, 0.25f);

		for (u32 i = 0; i < actionbar_buttons.size(); i++)
		{
			gui_draw_button(actionbar_buttons[i]);
		}
	}

	// draw action bar text
	if (actionbar_msg.seconds_since_start < actionbar_msg.show_seconds)
	{
		u32 width = font_get_text_width(inconsolata_font, actionbar_msg.msg, 0.25f);

		gui_draw_text(inconsolata_font, actionbar_msg.msg, graphics_projection_width / 2 - width / 2, graphics_projection_height / 7, 0.25f);
	}

	// draw combat log
	u32 log_box_height = 300;
	gui_draw_image(asset_manager_get_image("combat_log_bg"), 10, 20, 400, log_box_height);

	float scale = 0.15f;
	u32 log_end = combat_log_text->len - 1;
	u32 log_y = 0;
	u32 log_text_y_pad = 30;
	for (i32 i = combat_log_text->len - 2; i >= 0; i--)
	{
		char c = combat_log_text->raw[i];

		if (c == '\n' || i == 0)
		{
			u32 start = i;

			if(c == '\n')
			{
				start += 1;
				log_end -= 1;
			}

			gui_draw_text(inconsolata_font, combat_log_text->raw + start, log_end - i, 20, 30 + log_y, scale);
			log_y += log_text_y_pad;
			log_end = i - 2;

			if(log_y > log_box_height - log_text_y_pad) break;
		}
	}
}

void actionbar_set_msg(char* msg, float show_seconds)
{
	// @Todo: flash red when setting
	dynstr_clear(actionbar_msg.msg);
	dynstr_append_str(actionbar_msg.msg, msg);
	actionbar_msg.show_seconds = show_seconds;
	actionbar_msg.seconds_since_start = 0.0f;
}

void actionbar_combatlog_add(char* format, ...)
{
	if(!ENABLE_COMBAT_LOG) return;

	va_list args;
	va_start(args, format);

	dynstr_append_va(combat_log_text, format, args);
	
	// @Todo: use char append?
	dynstr_append_str(combat_log_text, "\n");

	va_end(args);
}

void action_move_mode()
{
	current_action_mode = ACTION_MODE_MOVE;

	u32 last_target_index = 0;
	vec3 target;

	while(actions[0].get_next_target(selected_entity, &last_target_index, &target))
	{
		poses.push_back(target);
	}
}

void action_shoot_mode()
{
	current_action_mode = ACTION_MODE_SHOOT;

	for(u32 x = 0; x < map_max_x; x++)
	{
		for (u32 z = 0; z < map_max_z; z++)
		{
			vec3 pos = vec3(x, 0.0f, z);

			if (map_has_los(selected_entity->pos, pos))
			{
				poses.push_back(pos);
			}
		}
	}
}

void action_throw_mode()
{
	current_action_mode = ACTION_MODE_THROW;
}

void action_end_turn()
{
	selected_entity->ap = 0;
	selected_entity = NULL;
	current_action_mode = ACTION_MODE_SELECT_UNITS;
}