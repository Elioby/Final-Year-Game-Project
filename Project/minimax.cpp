#include "minimax.h"

#include "action.h"

action_evaluation minimax_search(action_evaluation node, u32 depth, team maximizing_team, team current_team)
{
	// @Todo: check if the game is over at this point? although i doubt we ever get that far
	if(depth == 0) return node;

	bool maximizing = maximizing_team == current_team;

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

			}
		}
	}

	if(maximizing)
	{
		value = glm::max(value, minimax_search(child, depth - 1, false));
		return value;
	}
	else
	{
		float value = +FLT_MAX;
		value = glm::min(value, minimax_search(child, depth - 1, true));
		return value;
	}
}

//function minimax(node, depth, maximizingPlayer) is
//	if depth = 0 or node is a terminal node then
//	return the heuristic value of node
//	if maximizingPlayer then
//	value : = −∞
//		for each child of node do
//			value : = max(value, minimax(child, depth − 1, FALSE))
//			return value
//	else (*minimizing player *)
//	value : = +∞
//		for each child of node do
//			value : = min(value, minimax(child, depth − 1, TRUE))
//			return value
//