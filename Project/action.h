#pragma once

#include "general.h"
#include "entity.h"

typedef enum action_mode
{
	ACTION_MODE_SELECT_UNITS,
	ACTION_MODE_MOVE,
	ACTION_MODE_SHOOT,
	ACTION_MODE_THROW
} action_mode_t;

struct action_undo_data { };

struct action_evaluation
{
	// the action taken
	action* action;

	// the evaluation function after the action is taken, will be FLT_MIN if invalid
	float eval;
	vec3 target;

	// whatever data we need to undo this temporary action
	action_undo_data undo_data;

	// if we can actually use the action or not
	bool valid;
};

struct action
{
	char* name;
	void(*perform)(entity* ent, vec3 target);
	action_evaluation(*evaluate)(entity* ent);
};

extern std::vector<action> actions;

extern action action_nothing;
extern action_mode current_action_mode;

void action_init();
void action_update();