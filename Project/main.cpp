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

float evaluate_cover(team team)
{
	debug_timer_start("EVAL_BOARD_COVER");
	float eval = 0.0f;

	// @Speed: this is n^2

	// @Todo: abstract this out for use in UI code as well as AI code
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* enemy = entities[i];
		if(enemy->team != team)
		{
			for(u32 j = 0; j < entities.size(); j++)
			{
				entity* friendly = entities[j];
				if(friendly->team == team)
				{
					bool has = map_has_los(enemy, friendly);

					if(has)
					{
						// calculate the chance our enemy can hit us and subtract from eval (it's our disadvantage)
						float enemy_hit_friendly = map_get_los_angle(enemy, friendly);
						eval -= enemy_hit_friendly;

						// calculate the chance we can hit our enemy and add it to our eval (it's our advantage)
						float friendly_hit_enemy = map_get_los_angle(friendly, enemy);
						eval += friendly_hit_enemy;
					}
				}
			}
		}
	}
	debug_timer_end("EVAL_BOARD_COVER");

	return eval;
}

float evaluate_health(team team)
{
	float eval = 0.0f;
	
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* ent = entities[i];

		float entity_eval = ent->health / ent->max_health;

		// @Consider: how important is having them alive or dead? id say pretty important, on top of how much HP they have (they'll get another action next turn)
		entity_eval += ent->dead ? -1.0f : 1.0f;
		
		// if they're on our team, its good for them to be alive, bad for dead, and good for more hp. the opposite is true if they're an enemy
		if(ent->team == team) eval += entity_eval;
		else eval -= entity_eval;
	}

	return eval;
}

float evaluate_board(team team)
{
	debug_timer_start("EVAL_BOARD");

	float eval = 0.0f;

	eval += evaluate_cover(team);
	eval += evaluate_health(team);

	debug_timer_end("EVAL_BOARD");
	return eval;
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
	eval.eval = evaluate_board(ent->team);

	return eval;
}

void action_perform_move(entity* ent, vec3 target)
{
	ent->pos = target;
}

action_evaluation action_evaluate_move(entity* ent)
{
	vec3 original_position = ent->pos;

	vec3 best_target = vec3();
	float best_move_eval = -FLT_MAX;

	for(u32 x = 0; x < map_max_x; x++)
	{
		for(u32 z = 0; z < map_max_z; z++)
		{
			vec3 move_target = vec3(x, 0, z);

			if(map_is_cover_at_block(move_target)) continue;
			
			if(map_get_entity_at_block(move_target) != NULL) continue;

			ent->pos = move_target;

			float move_eval = evaluate_board(ent->team);

			if(move_eval > best_move_eval)
			{
				best_target = move_target;
				best_move_eval = move_eval;
			}
		}
	}

	action_evaluation eval = {0};

	if(best_move_eval > 0)
	{
		eval.target = best_target;
		eval.eval = best_move_eval;
		eval.valid = true;
	}
	else
	{
		eval.eval = -FLT_MAX;
		eval.valid = false;
	}

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
	float highest_eval = -FLT_MAX;

	// find the best entity to shoot
	for(u32 i = 0; i < entities.size(); i++)
	{
		entity* target_ent = entities[i];

		if(!entity_is_same_team(target_ent, ent))
		{
			if(!map_has_los(ent, target_ent)) continue;

			i32 original_hp = target_ent->health;

			action_perform_shoot(ent, target_ent, true);

			// @Todo: talk to frank about this. We have a chance to hit the shot, how should this effect the evaluation?
			float eval = evaluate_board(ent->team);
			
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
		eval.target = highest_eval_ent->pos;
		eval.valid = true;
		eval.eval = highest_eval;
	}
	else
	{
		eval.valid = false;
		eval.eval = -FLT_MAX;
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

// @Todo: consider AP
void do_ai(entity* ent)
{
	printf("AI entity %i\n", ent->id);
	
	debug_timer_start("DO_AI_ENTITY");
	debug_timer_reset("HAS_LOS_CHECK");

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

	best_action.perform(ent, best_eval.target);

	debug_timer_finalize("HAS_LOS_CHECK");
	debug_timer_finalize("DO_AI_ENTITY");
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

	// draw selected tile
	graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(input_mouse_block_pos, 0.0f, vec3(1.0f), vec3(1.0f, 0.1f, 1.0f)));

	// draw selected entity
	if(selected_entity)
	{
		graphics_draw_mesh(asset_manager_get_mesh("cube"), graphics_create_model_matrix(selected_entity->pos, radians(0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(1.0f, 0.05f, 1.0f)));
	}

	map_draw();

	// draw action bar
	actionbar_draw();
}

