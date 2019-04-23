#include "dynqueue.h"

dynqueue* dynqueue_create(u64 initial_size, u32 element_size)
{
	return (dynqueue*) dynarray_create(initial_size, element_size);
}

void dynqueue_destroy(dynqueue* queue)
{
	dynarray_destory(queue);
}

void* dynqueue_front(dynqueue* queue)
{
	debug_assert(queue->len > 0, "Tried to get the front of an empty queue");

	return dynarray_get(queue, 0);
}

void dynqueue_push(dynqueue* queue, void* data)
{
	dynarray_add(queue, data);
}

u32 x = 0;

void dynqueue_pop(dynqueue* queue)
{
	debug_assert(queue->len, "Tried to remove the front of an empty queue");
	dynarray_remove(queue, 0);

	x = max(queue->len, x);

	printf("%i\n", x);
}