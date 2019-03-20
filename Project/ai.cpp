#include "ai.h"

#include "entity.h"
#include "action.h"
#include "board_eval.h"
#include "turn.h"
#include "minimax.h"

action_evaluation action_evaluate(action act, entity* ent)
{
	action_evaluation best = {0};

	best.eval = -FLT_MAX;
	best.valid = false;

	u32 last_index = 0;
	vec3 target;
	while(act.get_next_target(ent, &last_index, &target))
	{
		action_undo_data* undo_data = act.gather_undo_data(ent, target);
		act.perform(ent, target, true);
		float eval = board_evaluate(ent->team);
		act.undo(ent, undo_data);

		// @Todo: instead of freeing we should just pop something off a stack
		free(undo_data);

		if(eval >= best.eval)
		{
			best.eval = eval;
			best.action = act;
			best.target = target;
			best.valid = true;
		}
	}

	return best;
}

// @Todo: consider AP
void ai_perform_entity(entity* ent)
{
	team team = ent->team;

	// start with nothing action, anything better than doing nothing we do
	action best_action = action_nothing;
	action_evaluation best_eval = action_evaluate(best_action, ent);

	board_eval_build_cache();

	best_eval = minimax_search(ent, best_eval, 2, team, team, -FLT_MAX, +FLT_MAX);

	board_eval_destroy_cache();

	printf("Entity %i performing action %s\n", ent->id, best_eval.action.name);

	best_eval.action.perform(ent, best_eval.target, false);
}

void ai_perform_team(team team)
{
	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* ent = entities[i];

		if (ent->team == team)
		{
			ai_perform_entity(ent);
		}
	}
}