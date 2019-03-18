#pragma once

#include "general.h"
#include "team.h"

float evaluate_cover(team team);
float evaluate_health(team team);
float evaluate_board(team team);

void board_eval_destroy_cache();
void board_eval_build_cache(team team);