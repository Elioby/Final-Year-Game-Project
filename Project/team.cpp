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

team team_get_opposite(team team)
{
	return team == TEAM_FRIENDLY ? TEAM_ENEMY : TEAM_FRIENDLY;
}