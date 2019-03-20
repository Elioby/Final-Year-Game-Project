#pragma once

#include "general.h"
#include "team.h"

float board_evaluate(team team);

void board_eval_destroy_cache();
void board_eval_build_cache(team team);