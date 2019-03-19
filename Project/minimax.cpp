#include "minimax.h"

#include <math.h>

#include "action.h"

float minimax_search(action_evaluation node, u32 depth, bool maximizing)
{
	// @Todo: check if the game is over at this point? although i doubt we ever get that far
	if(depth == 0) return node.eval;

	float value = -FLT_MAX;

	if(maximizing)
	{
		value = max(value, minimax_search(child, depth - 1, !maximizing));
		return ;
	}
	else
	{
		return min(value, )
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