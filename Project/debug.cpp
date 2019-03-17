#include "debug.h"

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#include <vector>

#include "general.h"

struct timer
{
	char* id;
	u64 current_start;
	u64 accumulated;
	u64 count;
};

std::vector<timer*> timers;

u64 get_current_time_ns()
{
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);

	u64 seconds = (u64) ts.tv_sec;

	return seconds * 1000000000 + (u64) ts.tv_nsec;
}

i32 get_timer_index(char* timer_id)
{
	i32 timer_array_index = 0;

	for(; timer_array_index < timers.size(); timer_array_index++)
	{
		timer* timer = timers[timer_array_index];
		if(strcmp(timer->id, timer_id) == 0) return timer_array_index;
	}

	return -1;
}

void debug_timer_start(char* timer_id)
{
	i32 timer_index = get_timer_index(timer_id);

	timer* tim;
	
	if(timer_index < 0)
	{
		u32 id_len = strlen(timer_id);
		tim = (timer*) calloc(1, sizeof(timer) + id_len + 1);
		tim->id = (char*) tim + sizeof(timer);
		memcpy(tim->id, timer_id, id_len);
		timers.push_back(tim);
	}
	else
	{
		tim = timers[timer_index];
	}

	tim->current_start = get_current_time_ns();
}

void debug_timer_end(timer* tim, u64 current_time)
{
	assert(tim->current_start && "Timer must be started for you to end it");

	tim->accumulated += current_time - tim->current_start;

	tim->count++;
	tim->current_start = 0;
}

void debug_timer_end(char* timer_id)
{
	u64 time = get_current_time_ns();
	debug_timer_end(timers[get_timer_index(timer_id)], time);
}

void debug_timer_reset(char* timer_id)
{
	i32 tim_index = get_timer_index(timer_id);

	if(tim_index >= 0)
	{
		timer* tim = timers[tim_index];
		timers.erase(timers.begin() + tim_index);
		free(tim);
	}
}

void debug_timer_finalize(char* timer_id)
{
	u64 current_time = get_current_time_ns();

	i32 tim_index = get_timer_index(timer_id);

	assert(tim_index >= 0 && "Timer not found");

	timer* tim = timers[tim_index];

	if(tim->current_start != 0) debug_timer_end(tim, current_time);

	printf("TIMER %s TOOK %f MS TO RUN %i TIMES\n", timer_id, (tim->accumulated) / 1000000.0, tim->count);

	debug_timer_reset(timer_id);
}