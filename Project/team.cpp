#include "team.h"

char* team_get_name(team team)
{
	switch (team)
	{
		case TEAM_FRIENDLY: return "Friendly";
		case TEAM_ENEMY: return "Enemy";
	}

	return "unknown team";
}

team get_opposite_team(team team)
{
	return team == TEAM_FRIENDLY ? TEAM_ENEMY : TEAM_FRIENDLY;
}