#pragma once

#include "general.h"
#include "action.h"

action_evaluation minimax_search(action_evaluation parent, u32 depth, team maximizing_team, team current_team);