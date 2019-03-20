#include "minimax.h"

#include "board_eval.h"

action_evaluation minimax_search(action_evaluation parent, u32 depth, team maximizing_team, team current_team)
{
	// @Todo: check if the game is over at this point? although i doubt we ever get that far
	if(depth == 0) return parent;

	bool maximizing = maximizing_team == current_team;
	team opposite_team = team_get_opposite(current_team);

	action_evaluation chosen_act = { 0 };

	if(maximizing) chosen_act.eval = -FLT_MAX;
	else chosen_act.eval = +FLT_MAX;

	u32 x = 0;

	for(u32 entity_index = 0; entity_index < entities.size(); entity_index++)
	{
		// @Speed: keep a list of friendly and enemy entities?
		entity* ent = entities[entity_index];

		if(ent->team != current_team) continue;

		for (u32 action_index = 0; action_index < actions.size(); action_index++)
		{
			action act = actions[action_index];

			u32 last_target_index = 0;
			vec3 target;

			while(act.get_next_target(ent, &last_target_index, &target))
			{
				action_undo_data* undo_data = act.gather_undo_data(ent, target);

				act.perform(ent, target, true);

				float eval = board_evaluate(maximizing_team);

				if(maximizing)
				{
					if(eval >= chosen_act.eval)
					{
						chosen_act.eval = eval;
						// @Todo: we're getting a pointer to something on the stack here..
						chosen_act.action = act;
						chosen_act.valid = true;
						chosen_act.target = target;
					}

					action_evaluation best_child_eval = minimax_search(chosen_act, depth - 1, maximizing_team, opposite_team);

					if(best_child_eval.eval >= chosen_act.eval)
					{
						chosen_act = best_child_eval;
					}
				}
				else
				{
					if (eval <= chosen_act.eval)
					{
						chosen_act.eval = eval;
						// @Todo: we're getting a pointer to something on the stack here..
						chosen_act.action = act;
						chosen_act.valid = true;
						chosen_act.target = target;
					}
					
					action_evaluation worst_child_eval = minimax_search(chosen_act, depth - 1, maximizing_team, opposite_team);

					if (worst_child_eval.eval >= chosen_act.eval)
					{
						chosen_act = worst_child_eval;
					}
				}

				act.undo(ent, undo_data);
				free(undo_data);
			}
		}
	}

	return chosen_act;
}

//function minimax(node, depth, maximizingPlayer) is
//	if depth = 0 or node is a terminal node then
//	return the heuristic value of node
//	if maximizingPlayer then
//	value : = −∞
//		for each child of node do
//			value : = max(value, minimax(child, depth − 1, FALSE))
//		return value
//	else (*minimizing player *)
//	value : = +∞
//		for each child of node do
//			value : = min(value, minimax(child, depth − 1, TRUE))
//		return value
//