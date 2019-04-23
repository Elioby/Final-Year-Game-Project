#include "minimax.h"

#include "board_eval.h"

action_evaluation minimax_search(entity* ent, action_evaluation parent, u32 start_depth, u32 depth, team maximizing_team, team current_team, float alpha, float beta)
{
	// @Todo: check if the game is over at this point? although i doubt we ever get that far
	if(depth == 0)
	{
		action_evaluation eval = { 0 };
		eval.eval = board_evaluate(maximizing_team);
		return eval;
	}

	bool maximizing = maximizing_team == current_team;
	team opposite_team = team_get_opposite(current_team);

	action_evaluation chosen_act = { 0 };
	chosen_act.valid = false;

	float extreme;

	if(maximizing) extreme = -FLT_MAX;
	else extreme = +FLT_MAX;

	evaluation extreme_eval = {0};
	extreme_eval.total = extreme;

	chosen_act.eval = extreme_eval;

	for(u32 action_index = 0; action_index < stack_array_length(actions); action_index++)
	{
		action act = actions[action_index];

		u32 last_target_index = 0;
		vec3 target;

		while(act.get_next_target(ent, &last_target_index, &target))
		{
			bool cutoff = false;
			action_undo_data* undo_data = act.gather_undo_data(ent, target);

			act.perform(ent, target, true);

			if(maximizing)
			{
				action_evaluation best_child_eval = minimax_search(ent, chosen_act, start_depth, depth - 1, maximizing_team, opposite_team, alpha, beta);

				if(best_child_eval.eval.total > chosen_act.eval.total)
				{
					chosen_act.eval = best_child_eval.eval;

					if(depth == start_depth)
					{
						chosen_act.valid = true;
						chosen_act.action = act;
						chosen_act.target = target;
					}
				}

				if(alpha < chosen_act.eval.total) alpha = chosen_act.eval.total;

				if(alpha >= beta) cutoff = true; // beta cut off
			}
			else
			{
				action_evaluation worst_child_eval = minimax_search(ent, chosen_act, start_depth, depth - 1, maximizing_team, opposite_team, alpha, beta);

				if(worst_child_eval.eval.total < chosen_act.eval.total)
				{
					chosen_act = worst_child_eval;
				}

				if(beta > chosen_act.eval.total) beta = chosen_act.eval.total;

				if(alpha >= beta) cutoff = true; // alpha cut off
			}

			act.undo(ent, undo_data);
			free(undo_data);

			if(cutoff) goto out_of_node_search;
		}
	}

	out_of_node_search:

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