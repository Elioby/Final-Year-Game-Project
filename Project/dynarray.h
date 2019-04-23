#pragma once

#include "general.h"

struct dynarray
{
	u32 len;
};

// we seperate this so you don't have to see our internal representations of stuff
struct dynarray_internal : dynarray
{
	u64 buf_len;
	u32 element_size;

	u8* data;
};

dynarray* dynarray_create(u64 initial_len, u32 element_size);
void dynarray_destory(dynarray* arr);

void dynarray_clear(dynarray* arr);

void* dynarray_get(dynarray* arr, u64 index);
void* dynarray_add(dynarray* arr, void* data);
void dynarray_remove(dynarray* arr, u64 index);