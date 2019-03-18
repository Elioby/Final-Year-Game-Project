#include "ai.h"

#include "entity.h"
#include "action.h"
#include "board_eval.h"

// @Todo: consider AP
void ai_perform_entity(entity* ent)
{
	team team = ent->team;

	// start with nothing action, anything better than doing nothing we do
	action best_action = action_nothing;
	action_evaluation best_eval = best_action.evaluate(ent);

	board_eval_build_cache(ent->team);

	printf("Evaluation for action %s is %f\n", best_action.name, best_eval.eval);

	for (u32 i = 0; i < actions.size(); i++)
	{
		action act = actions[i];

		action_evaluation eval = act.evaluate(ent);

		printf("Evaluation for action %s is %f\n", act.name, eval.eval);

		if (eval.valid && eval.eval >= best_eval.eval)
		{
			best_action = act;
			best_eval = eval;
		}
	}

	board_eval_destroy_cache();

	printf("Entity %i performing action %s\n", ent->id, best_action.name);

	best_action.perform(ent, best_eval.target);
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