#pragma once

#include "general.h"

#define HASHTABLE_DEFAULT_SIZE 20

struct hashtable
{
	u64 len;
};

s32 hashtable_hash_u32(u32 x);
s32 hashtable_hash_str(char* nullterm_str);

hashtable* hashtable_create(u64 initial_size, u32 element_size);
void hashtable_destroy(hashtable* table);

void* hashtable_get(hashtable* table, s32 key);
void hashtable_put(hashtable* table, s32 key, void* value);