#include "actionbar.h"

#include "graphics.h"
#include "assets.h"
#include "action.h"
#include "entity.h"

struct action_bar_msg
{
	dynstr* msg;
	float show_seconds;
	float seconds_since_start;
};

std::vector<button*> action_bar_buttons;

action_bar_msg bar_msg = { dynstr_new(20) };

dynstr* ap_text = dynstr_new(9);
dynstr* combat_log_text = dynstr_new(200);

void action_move_mode();
void action_shoot_mode();
void action_throw_mode();

void actionbar_init()
{
	button* move_button = gui_create_button();
	move_button->width = graphics_projection_width / 20;
	move_button->height = move_button->width;
	move_button->x = graphics_projection_width / 2 - (move_button->width * 4) / 2;
	move_button->y = move_button->height / 8;
	move_button->icon_img = action_move_image;
	move_button->bg_img = action_image;
	move_button->hover_bg_img = action_hover_image;
	move_button->click_callback = action_move_mode;
	action_bar_buttons.push_back(move_button);

	button* shoot_button = gui_create_button();
	shoot_button->width = graphics_projection_width / 20;
	shoot_button->height = shoot_button->width;
	shoot_button->x = graphics_projection_width / 2 - shoot_button->width / 2;
	shoot_button->y = shoot_button->height / 8;
	shoot_button->icon_img = action_shoot_image;
	shoot_button->bg_img = action_image;
	shoot_button->hover_bg_img = action_hover_image;
	shoot_button->click_callback = action_shoot_mode;
	action_bar_buttons.push_back(shoot_button);

	button* throw_button = gui_create_button();
	throw_button->width = graphics_projection_width / 20;
	throw_button->height = throw_button->width;
	throw_button->x = graphics_projection_width / 2 + (throw_button->width * 2) / 2;
	throw_button->y = throw_button->height / 8;
	throw_button->icon_img = action_throw_image;
	throw_button->bg_img = action_image;
	throw_button->hover_bg_img = action_hover_image;
	throw_button->click_callback = action_throw_mode;
	action_bar_buttons.push_back(throw_button);
}

void actionbar_update(float dt)
{
	bar_msg.seconds_since_start += dt;
}

void actionbar_draw()
{
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
			gui_draw_image(action_bar_top_bg_image, graphics_projection_width / 2 - bar_width / 2, graphics_projection_height - bar_height, bar_width, bar_height);

			// bar text
			float scale = 0.5f;
			u32 text_width = (u32)font_get_text_width(inconsolata_font, text, scale);
			u32 text_height = (u32)(128.0f * scale);

			gui_draw_text(inconsolata_font, text, graphics_projection_width / 2 - text_width / 2, graphics_projection_height - text_height, scale);
		}

		// draw bottom action bar
		gui_draw_image(action_bar_bg_image, graphics_projection_width / 4, 0, graphics_projection_width / 2, graphics_projection_width / 30);

		dynstr_clear(ap_text);

		dynstr_append(ap_text, "AP: %i / %i", selected_entity->ap, selected_entity->max_ap);
		gui_draw_text(inconsolata_font, ap_text, graphics_projection_width / 4 + 15, 15, 0.25f);

		for (u32 i = 0; i < action_bar_buttons.size(); i++)
		{
			gui_draw_button(*action_bar_buttons[i]);
		}
	}

	// draw action bar text
	if (bar_msg.seconds_since_start < bar_msg.show_seconds)
	{
		u32 width = font_get_text_width(inconsolata_font, bar_msg.msg, 0.25f);

		gui_draw_text(inconsolata_font, bar_msg.msg, graphics_projection_width / 2 - width / 2, graphics_projection_height / 7, 0.25f);
	}

	// draw combat log
	u32 log_box_height = 300;
	gui_draw_image(combat_log_bg_image, 10, 20, 400, log_box_height);

	float scale = 0.15f;
	u32 log_end = combat_log_text->len;
	u32 log_y = 0;
	u32 log_text_y_pad = 30;
	for (i32 i = combat_log_text->len - 2; i >= 0; i--)
	{
		char c = combat_log_text->raw[i];

		if (c == '\n' || i == 0)
		{
			gui_draw_text(inconsolata_font, combat_log_text->raw + i, log_end - i, 20, 30 + log_y, scale);
			log_y += log_text_y_pad;
			log_end = i + 1;

			if(log_y > log_box_height - log_text_y_pad) break;
		}
	}
}

void actionbar_set_msg(char* msg, float show_seconds)
{
	// @Todo: flash red when setting
	dynstr_clear(bar_msg.msg);
	dynstr_append_str(bar_msg.msg, msg);
	bar_msg.show_seconds = show_seconds;
	bar_msg.seconds_since_start = 0.0f;
}

void actionbar_combatlog_add(char* format, ...)
{
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
}

void action_shoot_mode()
{
	current_action_mode = ACTION_MODE_SHOOT;
}

void action_throw_mode()
{
	current_action_mode = ACTION_MODE_THROW;
}