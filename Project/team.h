#pragma once

#include "general.h"

typedef enum team
{
	TEAM_FRIENDLY,
	TEAM_ENEMY
} team_t;

char* team_get_name(team team);