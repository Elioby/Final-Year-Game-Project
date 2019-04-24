#include "debug.h"

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

#include <vector>
#include <signal.h>

#include "general.h"
#include "window.h"

#define DEBUG_PRINT_ALLOC true

u64 debug_total_allocated = 0;

void debug_assert(bool assert, char* message)
{
	if(!assert)
	{
		if(!window_error_message_box(message))
		{
			assert("Failed to open message box on failed assert...");
		}
		else
		{
			assert(false);
		}
	}
}

void* debug_malloc(size_t size)
{
#if DEBUG_PRINT_ALLOC
	printf("Allocating %i\n", size);

	debug_total_allocated += size;
#endif

	return malloc(size);
}

void* debug_calloc(size_t count, size_t size)
{
#if DEBUG_PRINT_ALLOC
	printf("Callocating %i\n", size * count);

	debug_total_allocated += size * count;
#endif
	
	return calloc(count, size);
}

void debug_print_total_allocated()
{
#if DEBUG_PRINT_ALLOC
	printf("Total allocated so far: %i\n", debug_total_allocated);
#endif
}