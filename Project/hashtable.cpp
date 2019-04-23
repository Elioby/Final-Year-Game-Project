#include "hashtable.h"

#include <stdio.h>

struct hashtable_entry
{
	// @Todo: make sure to set this to false when removing a value?
	// is this entry empty?
	bool full;

	s32 hash;
	void* value;
};

struct hashtable_internal : hashtable
{
	u64 buf_len;
	u32 element_size;

	hashtable_entry* entries;
};

// http://stackoverflow.com/a/12996028/1069256
s32 hashtable_hash_u32(u32 x)
{
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return x;
}

// source: http://www.cse.yorku.ca/~oz/hash.html (djb2 string hashes)
// requires a null terminated string
s32 hashtable_hash_str(char* nullterm_str)
{
	unsigned long hash = 5381;
	int c;

	while (c = *nullterm_str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

hashtable* hashtable_create(u64 initial_size, u32 element_size)
{
	debug_assert(initial_size > 0, "hashtable size must be > 0");

	hashtable_internal* itable = (hashtable_internal*) debug_calloc(1, sizeof(hashtable)
		+ (element_size + sizeof(hashtable_entry)) * initial_size);

	itable->len = 0;
	itable->buf_len = initial_size;
	itable->element_size = element_size;
	itable->entries = (hashtable_entry*) ((u8*) itable + sizeof(hashtable));

	return itable;
}

void hashtable_destroy(hashtable* table)
{
	free(table);
}

u32 hashtable_hashcode(hashtable* table, int key)
{
	hashtable_internal* internal_table = (hashtable_internal*) table;

	return key % internal_table->buf_len;
}

void* hashtable_get(hashtable* table, s32 hash)
{
	bool first_round = true;
	u32 start_hash_index = hashtable_hashcode(table, hash);
	u32 hash_index = start_hash_index;

	hashtable_internal* itable = (hashtable_internal*) table;

	hashtable_entry* entry;

	// move in array until an empty 
	while ((entry = (hashtable_entry*)((u8*) itable->entries + (hash_index * (sizeof(hashtable_entry) + itable->element_size))))->full)
	{
		// if we reach the start again, that means we didnt find the item
		if(!first_round && hash_index == start_hash_index) break;
		
		first_round = false;

		if(entry->hash == hash) return entry->value;

		++hash_index;

		// wrap around the table
		hash_index %= itable->buf_len;
	}

	return NULL;
}

void hashtable_put(hashtable* table, s32 hash, void* value)
{
	hashtable_internal* itable = (hashtable_internal*) table;

	debug_assert(itable->len < itable->buf_len, "No space left in the hashtable to put item");

	u32 start_hash_index = hashtable_hashcode(table, hash);
	u32 hash_index = start_hash_index;

	hashtable_entry* entry;

	// find next empty slot
	while ((entry = (hashtable_entry*) ((u8*) itable->entries + (hash_index * (sizeof(hashtable_entry) + itable->element_size))))->full)
	{
		++hash_index;

		// wrap back to the start of the table
		hash_index %= itable->buf_len;
	}

	table->len++;

	entry->full = true;
	entry->hash = hash;
	entry->value = (u8*) entry + sizeof(hashtable_entry);

	memcpy(entry->value, value, itable->element_size);
}

// @Todo:
//hashtable_item* hashtable_remove(hashtable* table, hashtable_item* item) 
//{
//	int hash_index = hashtable_hashcode(table, item->key);
//
//	//move in array until an empty
//	while(hashArray[hashIndex] != NULL) {
//
//		if(hashArray[hashIndex]->key == key) {
//			hashtable_item* temp = hashArray[hashIndex];
//
//			table[hashIndex] = dummyItem;
//			return temp;
//		}
//
//		//go to next cell
//		++hashIndex;
//
//		//wrap around the table
//		hashIndex %= SIZE;
//	}
//
//	return NULL;
//}