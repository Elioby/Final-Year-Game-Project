#pragma once

#include "general.h"

#define HASHTABLE_DEFAULT_SIZE 20

struct hashtable_item
{
	i32 key;
};

struct hashtable
{
	u32 size;
	u32 count;
	hashtable_item** items;
};

i32 hash_u32(u32 x);

hashtable* hashtable_create(u32 size);
void hashtable_destroy(hashtable* table);

hashtable_item* hashtable_get(hashtable* table, int key);
void hashtable_put(hashtable* table, hashtable_item* item);