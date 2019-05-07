#include "turn.h"

#include "entity.h"
#include "actionbar.h"
#include "ai.h"
#include "map.h"

u32 turn_number = 1;
team turn_team;

float simulate_game_count = 1000;

float total_games = 0;
float drawn_games = 0;
float friendly_won_games = 0;

void go_agane()
{
	if(total_games++ - drawn_games >= simulate_game_count)
	{
		printf("Finished games!\n");

		printf("Friendly winrate: %f\n Enemy winrate: %f\n", (friendly_won_games / (total_games - drawn_games)) * 100.0f, 100.0f - ((friendly_won_games / (total_games - drawn_games)) * 100.0f), (drawn_games / total_games) * 100.0f);

		while (true);
	}

	map_resetup();

	turn_number = 1;
	turn_team = TEAM_FRIENDLY;

	//turn_start(turn_team);
}

void turn_start(team team)
{
	if(turn_number > 100)
	{
		printf("Turn limit reached, draw!\n");
		actionbar_combatlog_add("Turn limit reached, draw!");
		drawn_games++;
		go_agane();
		return;
	}

	turn_team = team;

	bool enemies_alive = false;

	// reset ap
	for (u32 i = 0; i < entities->len; i++)
	{
		entity* ent = *((entity**) dynarray_get(entities, i));

		if (ent->team == team) ent->ap = ent->max_ap;
		else if(ent->health > 0) enemies_alive = true;
	}

	if(!enemies_alive)
	{
		if (team == TEAM_FRIENDLY) friendly_won_games++;

		printf("%s team won! %f\n", team_get_name(team), total_games - drawn_games);
		actionbar_combatlog_add("%s team won!", team_get_name(team));
		go_agane();
		return;
	}

	actionbar_combatlog_add("%s team's turn started", team_get_name(team));

	if (team == TEAM_ENEMY)
	{
		ai_perform_team(team, 4);
		turn_end();
	}
	else
	{
		return;
		ai_perform_team(team, 1);
		turn_end();
	}
}

void turn_end()
{
	if(action_get_action_mode() != ACTION_MODE_SELECT_UNITS)
	{
		action_switch_mode(ACTION_MODE_SELECT_UNITS);
	}

	u32 enemies_alive = 0;
	for (u32 i = 0; i < entities->len; i++)
	{
		entity* ent = *((entity**) dynarray_get(entities, i));

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
		entity* ent = *((entity**) dynarray_get(entities, i));

		if (ent->team == team)
		{
			team_entities++;

			if (ent->ap <= 0) team_entities_ended++;
		}
	}

	return team_entities == team_entities_ended;
}