#pragma once

#include "general.h"
#include "team.h"

extern u32 turn_number;
extern team turn_team;

void turn_start(team team);
void turn_end();

bool turn_is_complete(team team);