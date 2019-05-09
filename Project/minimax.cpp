#include "minimax.h"

#include "board_eval.h"
#include "map.h"

entity* closest_enemy(entity* ent)
{
	entity* closest = NULL;
	float closest_distance_squared = -1.0f;
	for(u32 i = 0; i < entities->len; i++)
	{
		entity* enemy = *(entity**) dynarray_get(entities, i);
		float distance = map_distance_squared(ent->pos, enemy->pos);

		if(enemy->team != ent->team && !enemy->dead && (closest == NULL || distance < closest_distance_squared))
		{
			closest = enemy;
			closest_distance_squared = distance;
		}
	}

	return closest;
}

double get_chanced_eval(action_evaluation eval)
{
	if (eval.eval.total > 0) return eval.eval.total * eval.chance;

	return eval.eval.total / eval.chance;
}

action_evaluation minimax_search(entity* ent, action_evaluation parent, u32 start_depth, u32 depth, team maximizing_team, team current_team, float alpha, float beta)
{
	bool maximizing = maximizing_team == current_team;

	entity* cur_ent = ent;

	//if (!maximizing) cur_ent = closest_enemy(ent);

	if(depth == 0 || ent->dead || !cur_ent)
	{
		action_evaluation eval = { 0 };
		eval.eval = board_evaluate(maximizing_team);
		eval.chance = 1.0;
		return eval;
	}

	team opposite_team = team_get_opposite(current_team);

	action_evaluation chosen_act = { 0 };
	chosen_act.valid = false;

	float extreme;

	if(maximizing) extreme = -FLT_MAX;
	else extreme = +FLT_MAX;

	evaluation extreme_eval = {0};
	extreme_eval.total = extreme;

	chosen_act.eval = extreme_eval;
	chosen_act.chance = 1.0;

	dynarray* targets = dynarray_create(200, sizeof(vec3));

	for(u32 action_index = 0; action_index < stack_array_length(actions); action_index++)
	{
		action act = actions[action_index];

		u32 last_target_index = 0;

		dynarray_clear(targets);
		act.get_targets(cur_ent, targets, true);

		for(u32 i = 0; i < targets->len; i++)
		{
			vec3 target = *(vec3*) dynarray_get(targets, i);

			bool cutoff = false;
			action_undo_data* undo_data = act.gather_undo_data(cur_ent, target);

			double chance = act.perform(cur_ent, target, true);

			if(maximizing)
			{
				action_evaluation best_child_eval = minimax_search(ent, chosen_act, start_depth, depth - 1, maximizing_team, opposite_team, alpha, beta);

				if(get_chanced_eval(best_child_eval) > get_chanced_eval(chosen_act))
				{
					chosen_act.eval = best_child_eval.eval;
					chosen_act.chance = best_child_eval.chance * chance;

					if(depth == start_depth)
					{
						chosen_act.valid = true;
						chosen_act.action = act;
						chosen_act.target = target;
					}
				}

				if(alpha < get_chanced_eval(chosen_act)) alpha = get_chanced_eval(chosen_act);

				if(alpha >= beta) cutoff = true; // beta cut off
			}
			else
			{
				action_evaluation worst_child_eval = minimax_search(ent, chosen_act, start_depth, depth - 1, maximizing_team, opposite_team, alpha, beta);

				if(get_chanced_eval(worst_child_eval) < get_chanced_eval(chosen_act))
				{
					chosen_act = worst_child_eval;

					chosen_act.chance = worst_child_eval.chance * chance;
				}

				if(beta > get_chanced_eval(chosen_act)) beta = get_chanced_eval(chosen_act);

				if(alpha >= beta) cutoff = true; // alpha cut off
			}

			act.undo(cur_ent, undo_data);
			free(undo_data);

			if(cutoff) goto out_of_node_search;
		}
	}

	out_of_node_search:

	dynarray_destory(targets);

	return chosen_act;
}