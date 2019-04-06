#include "ai.h"

#include "entity.h"
#include "action.h"
#include "board_eval.h"
#include "turn.h"
#include "minimax.h"

void ai_perform_entity(entity* ent)
{
	team team = ent->team;

	// start with nothing action, anything better than doing nothing we do
	action best_action;
	action_evaluation best_eval = { 0 };

	board_eval_build_cache();

	best_eval = minimax_search(ent, best_eval, 3, 3, team, team, -FLT_MAX, +FLT_MAX); 
	
	best_action = best_eval.action;

	if (!best_eval.valid) best_action = action_nothing;

	board_eval_destroy_cache();

	printf("Entity %i performing action %s on target (%f, %f)\n", ent->id, best_action.name, best_eval.target.x, best_eval.target.z);
	board_evaluation_print(best_eval.eval);

	best_action.perform(ent, best_eval.target, false);
}

void ai_perform_team(team team)
{
	for (u32 i = 0; i < entities.size(); i++)
	{
		entity* ent = entities[i];

		if (ent->team == team && !ent->dead)
		{
			ai_perform_entity(ent);
			ai_perform_entity(ent);
		}
	}
}