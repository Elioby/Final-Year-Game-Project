#pragma once

#include "general.h"
#include "entity.h"
#include "board_eval.h"

struct action_undo_data { };

struct action
{
	char* name;
	action_undo_data* (*gather_undo_data)(entity* ent, vec3 target);
	void (*perform)(entity* ent, vec3 target, bool temp);
	void (*undo)(entity* ent, action_undo_data* undo_data);

	bool (*get_next_target)(entity* ent, u32* last_index, vec3* result);
};

typedef enum action_mode
{
	ACTION_MODE_SELECT_UNITS,
	ACTION_MODE_MOVE,
	ACTION_MODE_SHOOT,
	ACTION_MODE_THROW
} action_mode_t;

struct action_evaluation
{
	// the action taken
	action action;

	// the evaluation function after the action is taken, will be FLT_MIN if invalid
	evaluation eval;
	vec3 target;

	// if we can actually use the action or not
	bool valid;
};

extern action actions[2];

extern action action_nothing;
extern action action_move;
extern action action_shoot;

extern action_mode current_action_mode;

void action_init();
void action_update();