#pragma once

void debug_assert(bool assert, char* message);

void* debug_malloc(size_t size);
void* debug_calloc(size_t count, size_t size);

void debug_print_total_allocated();