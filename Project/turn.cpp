#include "turn.h"

#include "entity.h"
#include "actionbar.h"
#include "ai.h"

u32 turn_number = 1;
team turn_team;

void turn_start(team team)
{
	turn_team = team;

	bool enemies_alive = false;

	// reset ap
	for (u32 i = 0; i < entities->len; i++)
	{
		entity* ent = (entity*) dynarray_get(entities, i);

		if (ent->team == team) ent->ap = ent->max_ap;
		else enemies_alive = true;
	}

	if(!enemies_alive)
	{
		actionbar_combatlog_add("%s team won!", team_get_name(team));
		return;
	}

	actionbar_combatlog_add("%s team's turn started", team_get_name(team));

	if (team == TEAM_ENEMY)
	{
		ai_perform_team(team);
		turn_end();
	}
}

void turn_end()
{
	u32 enemies_alive = 0;
	for (u32 i = 0; i < entities->len; i++)
	{
		entity* ent = (entity*) dynarray_get(entities, i);

		if (ent->team == turn_team) ent->ap = 0;
		else enemies_alive++;
	}

	turn_number++;

	actionbar_combatlog_add("%s team's turn ended", team_get_name(turn_team));

	team new_team = team_get_opposite(turn_team);

	if(enemies_alive > 0)
	{
		turn_start(new_team);
	}
	else
	{
		printf("Game over! %s team won!\n", team_get_name(turn_team));
	}
}

bool turn_is_complete(team team)
{
	u32 team_entities = 0;
	u32 team_entities_ended = 0;

	for (u32 i = 0; i < entities->len; i++)
	{
		entity* ent = (entity*) dynarray_get(entities, i);

		if (ent->team == team)
		{
			team_entities++;

			if (ent->ap <= 0) team_entities_ended++;
		}
	}

	return team_entities == team_entities_ended;
}