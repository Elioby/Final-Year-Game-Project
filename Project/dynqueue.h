#pragma once

#include "dynarray.h"

// we use the same data structures as dynarray
struct dynqueue : dynarray { };
struct dynqueue_internal : dynarray_internal { };

dynqueue* dynqueue_create(u32 initial_size, u32 element_size);
void dynqueue_destroy(dynqueue* queue);

void* dynqueue_front(dynqueue* queue);

void dynqueue_push(dynqueue* queue, void* data);
void dynqueue_pop(dynqueue* queue);