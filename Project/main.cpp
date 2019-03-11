#include <stdio.h>

#include <glm/glm.hpp>

#include <bgfx/c99/bgfx.h>

#include "general.h"
#include "window.h"
#include "graphics.h"
#include "input.h"
#include "gui.h"
#include "asset_manager.h"
#include "map.h"
#include "entity.h"
#include "font.h"
#include "dynstr.h"
#include "actionbar.h"
#include "action.h"

void draw();
void update(float dt);
void camera_update(float dt);
void turn_init();
void action_init();

void turn_end();
// @Todo: temp
button* b;

void button_end_turn_all()
{
	turn_end();
}

int main()
{
	window_init();
	graphics_init(WINDOW_WIDTH, WINDOW_HEIGHT);
	input_init();
	gui_init();
	asset_manager_init();
	map_init();
	action_init();
	actionbar_init();
	turn_init();
	shader_init();

	b = gui_create_button();
	b->width = 100;
	b->height = 100;
	b->bg_img = asset_manager_get_image("action_button");
	b->hover_bg_img = asset_manager_get_image("action_button_hover");
	b->click_callback = button_end_turn_all;

	float lastTime = 0;
	float time, dt;

	shader_set_tint_uniform(vec4(1.0f, 0.0f, 0.0f, 1.0f));

	while(!glfwWindowShouldClose(window))
	{
		time = (float) glfwGetTime();
		dt = time - lastTime;
		lastTime = time;

		glfwPollEvents();
		update(dt);

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx_touch(0);

		bgfx_dbg_text_clear(0, false);

		bgfx_dbg_text_printf(0, 0, 0x0f, "Last frame time: %.2fms, FPS: %.0f", dt * 1000.0f, 1 / dt);

		draw();

		bgfx_frame(false);

		input_end_frame();
		gui_end_frame();
	}

	bgfx_shutdown();
	glfwTerminate();

	return 0;
}

u32 turn_number = 1;
team turn_team;

bool team_is_turn_over(team team)
{
	u32 team_entities = 0;
	u32 team_entities_ended = 0;

	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];

		if(entity->team == team)
		{
			team_entities++;

			if(entity->ap <= 0) team_entities_ended++;
		}
	}

	return team_entities == team_entities_ended;
}

char* team_get_name(team team)
{
	switch(team)
	{
		case TEAM_FRIENDLY: return "Friendly";
		case TEAM_ENEMY: return "Enemy";
	}

	return "unknown team";
}

void turn_start(team team);

void turn_init()
{
	turn_start(TEAM_FRIENDLY);
}

float evaluate_board()
{
	return 1.0f;
}

struct action_evaluation
{
	// the evaluation function after the action is taken, will be FLT_MIN if invalid
	float eval;
	vec3 target;

	// if we can actually use the action or not
	bool valid;
};

void action_perform_nothing(entity* ent, vec3 target) { }

action_evaluation action_evaluate_nothing(entity* ent)
{
	action_evaluation eval = {0};

	eval.valid = true;
	eval.eval = evaluate_board();

	return eval;
}

void action_perform_move(entity* ent, vec3 target)
{
	ent->pos = target;
}

action_evaluation action_evaluate_move(entity* ent)
{
	vec3 original_position = ent->pos;

	vec3 target = vec3(1.0f);

	ent->pos = target;

	action_evaluation eval = {0};

	eval.eval = evaluate_board() + 1.0f;
	eval.valid = true;

	ent->pos = original_position;

	return eval;
}

#define ACTION_SHOOT_DAMAGE 6

void action_perform_shoot(entity* ent, entity* target_ent, bool temp)
{
	entity_health_change(target_ent, ent, -ACTION_SHOOT_DAMAGE, temp);
}

void action_perform_shoot(entity* ent, vec3 target)
{
	entity* target_ent = map_get_entity_at_block(target);

	action_perform_shoot(ent, target_ent, false);
}

action_evaluation action_evaluate_shoot(entity* ent)
{
	entity* highest_eval_ent = NULL;
	float highest_eval = FLT_MIN;

	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* target_ent = entities[i];

		if(!entity_is_same_team(target_ent, ent))
		{
			i32 original_hp = target_ent->health;

			action_perform_shoot(ent, target_ent, true);

			float eval = evaluate_board() + 2.0f;
			
			// if the dmg done was > previous hp, just heal back the previous health (otherwise they gain health)
			i32 heal_amount = ACTION_SHOOT_DAMAGE;

			if(heal_amount > original_hp) heal_amount = original_hp;

			// heal them back up (it will also res them if they're dead, since this is temp mode)
			entity_health_change(target_ent, ent, heal_amount, true);

			if(eval > highest_eval)
			{
				highest_eval_ent = target_ent;
				highest_eval = eval;
			}
		}
	}

	action_evaluation eval = {0};

	if(highest_eval_ent)
	{
		eval.target = ent->pos;
		eval.valid = true;
	}
	else
	{
		eval.valid = false;
		eval.eval = FLT_MIN;
	}

	return eval;
}

struct action
{
	char* name;
	void(*perform)(entity* ent, vec3 target);
	action_evaluation(*evaluate)(entity* ent);
};

std::vector<action> actions;

action nothing_action;

void action_init()
{
	nothing_action = {0};
	nothing_action.name = "nothing";
	nothing_action.perform = action_perform_nothing;
	nothing_action.evaluate = action_evaluate_nothing;

	action move_action = {0};
	move_action.name = "move";
	move_action.perform = action_perform_move;
	move_action.evaluate = action_evaluate_move;
	actions.push_back(move_action);

	action shoot_action = {0};
	shoot_action.name = "shoot";
	shoot_action.perform = action_perform_shoot;
	shoot_action.evaluate = action_evaluate_shoot;
	actions.push_back(shoot_action);
}

void do_ai(entity* ent)
{
	printf("AI entity %i\n", ent->id);

	// start with nothing action, anything better than doing nothing we do
	action best_action = nothing_action;
	action_evaluation best_eval = best_action.evaluate(ent);

	for(u32 i = 0; i < actions.size(); i++)
	{
		action act = actions[i];

		action_evaluation eval = act.evaluate(ent);

		printf("Action %s: %f\n", act.name, eval.eval);

		if(eval.valid && eval.eval > best_eval.eval)
		{
			best_action = act;
			best_eval = eval;
		}
	}

	printf("Chosen action: %s\n", best_action.name);
}

void do_ai(team team)
{
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* ent = entities[i];

		if(ent->team == team)
		{
			do_ai(ent);
		}
	}
}

void turn_start(team team)
{
	turn_team = team;

	// reset ap
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];

		if(entity->team == team) entity->ap = entity->max_ap;
	}

	actionbar_combatlog_add("%s team's turn started", team_get_name(team));

	if(team == TEAM_ENEMY)
	{
		do_ai(team);
	}
}

void turn_end()
{
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* entity = entities[i];

		if(entity->team == turn_team) entity->ap = 0;
	}

	turn_number++;

	actionbar_combatlog_add("%s team's turn ended", team_get_name(turn_team));

	team new_team;

	if(turn_team == TEAM_FRIENDLY) new_team = TEAM_ENEMY;
	else new_team = TEAM_FRIENDLY;

	turn_start(new_team);
}

void update(float dt)
{
	camera_update(dt);
	input_update();

	gui_update();

	entity_update();
	action_update();
	actionbar_update(dt);

	// turn update
	if(team_is_turn_over(turn_team))
	{
		turn_end();
	}
}

void draw()
{
	b->x = graphics_projection_width - b->width - 30;
	b->y = graphics_projection_height - b->height - 30;
	gui_draw_button(b);

	// @Debug: debug los
	for(u32 i = 0; i < map_debug_los.size(); i++)
	{
		graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(map_debug_los[i], 0.0f, vec3(1.0f), vec3(1.0f, 0.25f, 1.0f)));
	}

	// draw selected tile
	graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(input_mouse_block_pos, 0.0f, vec3(1.0f), vec3(1.0f, 0.1f, 1.0f)));

	// draw selected entity
	if(selected_entity)
	{
		graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(selected_entity->pos, radians(0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.05f, 1.0f)));
	}

	// @Todo: move to map_draw
	// draw cover
	for(u32 i = 0; i < cover_list.size(); i++)
	{
		cover* cov = cover_list[i];
		graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(cov->pos, 0.0f, vec3(1.0f), vec3(1.0f, 2.0f, 1.0f)));
	}

	// @Todo: move to map_draw
	// draw entites and healthbars
	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* ent = entities[i];

		if(!ent->dead)
		{
			vec3 healthbox_aspect = vec3(1.0f, 1.0f / 3.0f, 1.0f);

			if (ent->health > 0) 
			{
				image* img;
				if(ent->team == TEAM_ENEMY) img = asset_manager_get_image("enemy_healthbar");
				else img = asset_manager_get_image("friendly_healthbar");

				graphics_draw_image(img, graphics_create_model_matrix(vec3(ent->pos.x + 0.033333f, ent->pos.y + 2.038f, ent->pos.z + 0.5f), 0.0f, vec3(1.0f),
					vec3((0.5f - 0.1f / 3.0f) * (ent->health / (float)ent->max_health), 0.1285f, 1.0f)));
			}

			graphics_draw_image(asset_manager_get_image("healthbox"), graphics_create_model_matrix(vec3(ent->pos.x, ent->pos.y + 2.0f, ent->pos.z + 0.5f), 0.0f, vec3(1.0f), healthbox_aspect * 0.5f));

			graphics_draw_mesh(ent->mesh, graphics_create_model_matrix(ent->pos, 0.0f, vec3(1.0f), vec3(1.0f)));
		}
	}

	// @Todo: move to map_draw
	// draw terrain
	graphics_draw_mesh(asset_manager_get_mesh("terrain"), graphics_create_model_matrix(vec3(0.0f), 0.0f, vec3(1.0f), vec3(1.0f)));

	// draw action bar
	actionbar_draw();
}

