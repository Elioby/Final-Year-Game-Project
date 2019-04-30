#include "actionbar.h"

#include "graphics.h"
#include "asset_manager.h"
#include "action.h"
#include "entity.h"
#include "map.h"
#include "dynarray.h"

#define ENABLE_COMBAT_LOG true

struct actionbar_msg_show
{
	dynstr* msg;
	float show_seconds;
	float seconds_since_start;
};

struct shot_target {
	button* button;
	entity* target_entity;
	float shot_chance;
};

u32 actionbar_width;

button* action_buttons[3];
dynarray* shoot_target_buttons = dynarray_create(10, sizeof(shot_target));

actionbar_msg_show actionbar_msg = { dynstr_create(20) };

dynstr* ap_text = dynstr_create(12);
dynstr* button_shoot_text = dynstr_create(12);
dynstr* combat_log_text = dynstr_create(200);

void action_move_mode(button* this_button);
void action_shoot_mode(button* this_button);
void action_throw_mode(button* this_button);
void action_end_turn(button* this_button);

void action_shoot_unit_button_click(button* this_button);
void action_shoot_unit_button_hover(button* this_button);

void actionbar_init()
{
	actionbar_width = (u32) (graphics_projection_width / 2.5f);

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
	action_buttons[0] = move_button;

	button* shoot_button = gui_create_button();
	shoot_button->width = button_width;
	shoot_button->height = button_width;
	shoot_button->x = graphics_projection_width / 2 - shoot_button->width / 2;
	shoot_button->y = shoot_button->height / 8;
	shoot_button->icon_img = asset_manager_get_image("action_shoot");
	shoot_button->bg_img = bg;
	shoot_button->hover_bg_img = hover;
	shoot_button->click_callback = action_shoot_mode;
	action_buttons[1] = shoot_button;

	/*button* throw_button = gui_create_button();
	throw_button->width = button_width;
	throw_button->height = button_width;
	throw_button->x = graphics_projection_width / 2 + (throw_button->width * 2) / 2;
	throw_button->y = throw_button->height / 8;
	throw_button->icon_img = asset_manager_get_image("action_throw");
	throw_button->bg_img = bg;
	throw_button->hover_bg_img = hover;
	throw_button->click_callback = action_throw_mode;
	action_buttons[2] = throw_button;*/

	button* end_button = gui_create_button();
	end_button->width = button_width;
	end_button->height = button_width;
	end_button->y = end_button->height / 8;
	end_button->icon_img = asset_manager_get_image("action_end");
	end_button->bg_img = bg;
	end_button->hover_bg_img = hover;
	end_button->click_callback = action_end_turn;
	action_buttons[2] = end_button;

	float padding = 0.5f;
	float button_width_padding = button_width * (1.0f + padding);

	u32 button_count = stack_array_length(action_buttons);

	u32 buttons_start = (u32) (graphics_projection_width / 2 - ((button_count * button_width) + ((button_count - 1) * button_width * padding)) / 2.0f);
	for(u32 i = 0; i < button_count; i++)
	{
		button* b = action_buttons[i];

		b->x = (u32) (buttons_start + button_width_padding * i);
	}
}

void actionbar_update(float dt)
{
	actionbar_msg.seconds_since_start += dt;
}

// @Todo: temp
std::vector<vec3> poses;

void actionbar_draw()
{
	// @Todo: why is this in action bar?
	for (u32 i = 0; i < poses.size(); i++)
	{
		vec3 ve = poses[i];
		ve.y = 0.15f;
		graphics_draw_mesh(asset_manager_get_mesh("plane"), graphics_create_model_matrix(ve, 90.0f, vec3(1.0f, 0.0f, 0.0f), vec3(0.5f)), vec4(0.0f, 0.492f, 0.901f, 0.3f));
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

			gui_draw_text(inconsolata_font, text, vec4(1.0f, 1.0f, 1.0f, 1.0f), graphics_projection_width / 2 - text_width / 2, graphics_projection_height - text_height, scale);
		}

		// draw bottom action bar
		gui_draw_image(asset_manager_get_image("actionbar_bg"), (graphics_projection_width / 2) - (actionbar_width / 2), 0,
			actionbar_width, graphics_projection_width / 30);

		dynstr_clear(ap_text);

		dynstr_append(ap_text, "AP: %i / %i", selected_entity->ap, selected_entity->max_ap);
		gui_draw_text(inconsolata_font, ap_text, vec4(0.0f, 0.0f, 0.0f, 1.0f), (graphics_projection_width / 2) - (actionbar_width / 2) + 15, 15, 0.25f);
	}

	// draw action buttons
	for (u32 i = 0; i < stack_array_length(action_buttons); i++)
	{
		button* b = action_buttons[i];
		b->visible = selected_entity != NULL;
		gui_draw_button(b);
	}

	// draw shoot target buttons
	for (u32 i = 0; i < shoot_target_buttons->len; i++)
	{
		shot_target* st = (shot_target*) dynarray_get(shoot_target_buttons, i);

		st->button->x = i * 40;
		
		gui_draw_button(st->button);

		dynstr_clear(button_shoot_text);
		dynstr_append(button_shoot_text, "%i%%", (int) st->shot_chance);

		gui_draw_text(inconsolata_font, button_shoot_text, vec4(1.0f, 0.0f, 0.0f, 1.0f), i * 40, 20, 0.1f);
	}

	// draw action bar text
	if (actionbar_msg.seconds_since_start < actionbar_msg.show_seconds)
	{
		u32 width = font_get_text_width(inconsolata_font, actionbar_msg.msg, 0.25f);

		gui_draw_text(inconsolata_font, actionbar_msg.msg, vec4(glm::min(0.8f - actionbar_msg.seconds_since_start * 0.75f, 1.0f), 0.0f, 0.0f, 
			glm::min(actionbar_msg.show_seconds - actionbar_msg.seconds_since_start, 1.0f)), graphics_projection_width / 2 - width / 2, graphics_projection_height / 7, 0.25f);
	}

	// draw combat log
	u32 log_box_height = 300;
	gui_draw_image(asset_manager_get_image("combat_log_bg"), 10, 20, 400, log_box_height);

	float scale = 0.15f;
	u32 log_end = combat_log_text->len - 1;
	u32 log_y = 0;
	u32 log_text_y_pad = 30;
	for (s32 i = combat_log_text->len - 2; i >= 0; i--)
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

			gui_draw_text(inconsolata_font, combat_log_text->raw + start, log_end - i, vec4(0.0f, 0.0f, 0.0f, 1.0f), 20, 30 + log_y, scale);
			log_y += log_text_y_pad;
			log_end = i - 2;

			if(log_y > log_box_height - log_text_y_pad) break;
		}
	}
}

void actionbar_set_msg(char* msg, float show_seconds)
{
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
	
	dynstr_append_char(combat_log_text, '\n');

	va_end(args);
}

void action_move_mode(button* this_button)
{
	current_action_mode = ACTION_MODE_MOVE;

	u32 last_target_index = 0;
	vec3 target;

	while(actions[0].get_next_target(selected_entity, &last_target_index, &target))
	{
		poses.push_back(target);
	}
}

void action_shoot_mode(button* this_button)
{
	dynarray_clear(shoot_target_buttons);

	u32 i = 0;
	vec3 target_pos;
	while(action_shoot.get_next_target(selected_entity, &i, &target_pos))
	{
		entity* target = map_get_entity_at_block(target_pos);

		button* b = gui_create_button();

		b->width = 40;
		b->height = 40;
		b->click_callback = action_shoot_unit_button_click;
		b->hover_callback = action_shoot_unit_button_hover;
		b->bg_img = asset_manager_get_image("action_button");
		b->hover_bg_img = asset_manager_get_image("action_button_hover");

		dynarray_add(shoot_target_buttons, &b);
	}

	if(shoot_target_buttons->len > 0)
	{
		current_action_mode = ACTION_MODE_SHOOT;
	}
	else
	{
		actionbar_set_msg("No targets to shoot", 3.0f);
	}
}

void action_throw_mode(button* this_button)
{
	current_action_mode = ACTION_MODE_THROW;
}

void action_end_turn(button* this_button)
{
	selected_entity->ap = 0;
	selected_entity = NULL;
	current_action_mode = ACTION_MODE_SELECT_UNITS;
}

void action_shoot_unit_button_click(button* this_button)
{
	u32 i;

	for (i = 0; i < shoot_target_buttons->len; i++)
	{
		button* b = *((button**)dynarray_get(shoot_target_buttons, i));
		if (b == this_button) break;
	}

	printf("%i\n", i);

	u32 j = 0;
	vec3 target_pos;
	for (u32 k = 0; k <= i; k++)
	{
		action_shoot.get_next_target(selected_entity, &j, &target_pos);
	}

	entity* target = map_get_entity_at_block(target_pos);

	action_shoot.perform(selected_entity, target_pos, false);

	if(target->dead)
	{
		dynarray_remove(shoot_target_buttons, i);

		// @Todo: we should really have a way of destroying buttons..
		this_button->visible = false;
	}
}

void action_shoot_unit_button_hover(button* this_button)
{
	u32 i;

	for(i = 0; i < shoot_target_buttons->len; i++)
	{
		button* b = *((button**) dynarray_get(shoot_target_buttons, i));
		if (b == this_button) break;
	}

	u32 j = 0;
	vec3 target_pos;
	for (u32 k = 0; k <= i; k++)
	{
		action_shoot.get_next_target(selected_entity, &j, &target_pos);
	}

	entity* target = map_get_entity_at_block(target_pos);

	if (this_button->hovering) targeted_entity = target;
	else if (targeted_entity == target) targeted_entity = NULL;
}