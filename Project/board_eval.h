#pragma once

#include "general.h"
#include "team.h"

struct evaluation
{
	float shot_chance;
	float health;
	float distance;
	float cover;

	float total;
};

evaluation board_evaluate(team team);
void board_evaluation_print(evaluation eval);

void board_eval_destroy_cache();
void board_eval_build_cache();