#include "debug.h"

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#include <vector>

#include "general.h"

std::vector<char*> timer_ids;
std::vector<u64> timer_starts;

u64 get_current_time_ms()
{
	time_t s;
	_timeb tstruct;

	time(&s);

	u64 seconds = (u64)s;

	_ftime(&tstruct);
	return seconds * 1000 + (u64)tstruct.millitm;
}

void debug_timer_start(char* timer_id)
{
	timer_ids.push_back(timer_id);
	timer_starts.push_back(get_current_time_ms());
}

void debug_timer_end(char* timer_id)
{
	u32 timer_array_index = 0;

	for(; timer_array_index < timer_ids.size(); timer_array_index++)
	{
		if(strcmp(timer_ids[timer_array_index], timer_id) == 0) break;

		assert(timer_array_index < timer_ids.size() - 1 && "Failed to find specified timer");
	}

	u64 timer_start = timer_starts[timer_array_index];

	timer_ids.erase(timer_ids.begin() + timer_array_index);
	timer_starts.erase(timer_starts.begin() + timer_array_index);

	printf("TIMER %s TOOK %i MS\n", timer_id, (get_current_time_ms() - timer_start));
}