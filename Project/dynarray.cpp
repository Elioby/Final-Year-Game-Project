#include "dynarray.h"

// @Todo: do we want to say new or create?
dynarray* dynarray_create(u64 initial_len, u32 element_size)
{
	debug_assert(initial_len >= 0, "A dynamic array must have an initial size >= 0");

	dynarray_internal* arr = (dynarray_internal*) debug_malloc(sizeof(dynarray_internal));

	arr->len = 0;
	arr->buf_len = initial_len;

	arr->element_size = element_size;
	arr->data = (u8*) debug_malloc(initial_len * element_size);

	return arr;
}

void dynarray_destory(dynarray* arr)
{
	dynarray_internal* internal_arr = (dynarray_internal*) arr;
	free(internal_arr->data);
	free(arr);
}

void dynarray_grow(dynarray* arr)
{
	dynarray_internal* internal_arr = (dynarray_internal*)arr;

	internal_arr->buf_len += internal_arr->buf_len / 2;

	internal_arr->data = (u8*) realloc(internal_arr->data, internal_arr->buf_len * internal_arr->element_size);
}

void dynarray_clear(dynarray* arr)
{
	arr->len = 0;
}

void* dynarray_get(dynarray* arr, u64 index)
{
	debug_assert(index < arr->len && index >= 0, "Dynamic array index out of bounds");

	dynarray_internal* internal_arr = (dynarray_internal*) arr;

	return internal_arr->data + (internal_arr->element_size * index);
}

void* dynarray_add(dynarray* arr, void* data)
{
	dynarray_internal* internal_arr = (dynarray_internal*) arr;

	if(internal_arr->len >= internal_arr->buf_len)
	{
		dynarray_grow(arr);
	}

	void* element = internal_arr->data + (internal_arr->len * internal_arr->element_size);

	memcpy(element, data, internal_arr->element_size);

	internal_arr->len++;

	return element;
}

void dynarray_remove(dynarray* arr, u64 index)
{
	dynarray_internal* internal_arr = (dynarray_internal*) arr;

	debug_assert(index < internal_arr->len, "Dynamic array remove index out of range");
	
	// move any later elements back
	if(index < internal_arr->len)
	{
		memcpy(internal_arr->data + (index * internal_arr->element_size), internal_arr->data + ((index + 1) * internal_arr->element_size),
			((arr->len - index) * internal_arr->element_size));
	}

	internal_arr->len--;
}