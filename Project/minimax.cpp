#include "minimax.h"

#include "board_eval.h"

action_evaluation minimax_search(entity* ent, action_evaluation parent, u32 depth, team maximizing_team, team current_team, float alpha, float beta)
{
	// @Todo: check if the game is over at this point? although i doubt we ever get that far
	if(depth == 0) return parent;

	bool maximizing = maximizing_team == current_team;
	team opposite_team = team_get_opposite(current_team);

	action_evaluation chosen_act = { 0 };
	chosen_act.valid = false;

	if(maximizing) chosen_act.eval = -FLT_MAX;
	else chosen_act.eval = +FLT_MAX;

	for(u32 action_index = 0; action_index < actions.size(); action_index++)
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
				if(eval > chosen_act.eval)
				{
					chosen_act.eval = eval;
					// @Todo: we're getting a pointer to something on the stack here..
					chosen_act.action = act;
					chosen_act.valid = true;
					chosen_act.target = target;
				}

				action_evaluation best_child_eval = minimax_search(ent, chosen_act, depth - 1, maximizing_team, opposite_team, alpha, beta);

				if(best_child_eval.eval > chosen_act.eval)
				{
					chosen_act.eval = best_child_eval.eval;
					chosen_act.action = act;
					chosen_act.valid = true;
					chosen_act.target = target;
				}

				if(alpha < chosen_act.eval) alpha = chosen_act.eval;

				if(alpha >= beta) goto out_of_node_search; // beta cut off
			}
			else
			{
				if (eval < chosen_act.eval)
				{
					chosen_act.eval = eval;
					// @Todo: we're getting a pointer to something on the stack here..
					chosen_act.action = act;
					chosen_act.valid = true;
					chosen_act.target = target;
				}
					
				action_evaluation worst_child_eval = minimax_search(ent, chosen_act, depth - 1, maximizing_team, opposite_team, alpha, beta);

				if (worst_child_eval.eval > chosen_act.eval)
				{
					chosen_act = worst_child_eval;
				}

				if(beta > chosen_act.eval) beta = chosen_act.eval;

				if(alpha >= beta) goto out_of_node_search; // alpha cut off
			}

			act.undo(ent, undo_data);
			free(undo_data);
		}
	}

	out_of_node_search:

	if (!chosen_act.valid) return parent;

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