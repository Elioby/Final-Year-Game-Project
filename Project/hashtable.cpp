#include "hashtable.h"

#include <stdio.h>

i32 hashtable_hash_u32(u32 x)
{
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	return x;
}

hashtable* hashtable_create(u32 size)
{
	debug_assert(size > 0, "hashtable size must be > 0");

	hashtable* table = (hashtable*) calloc(1, sizeof(hashtable) + sizeof(hashtable_item*) * size);

	table->size = size;
	table->items = (hashtable_item**)((char*)table + sizeof(hashtable));

	return table;
}

void hashtable_destroy(hashtable* table)
{
	free(table);
}

int hashtable_hashcode(hashtable* table, int key)
{
	return key % table->size;
}

hashtable_item* hashtable_get(hashtable* table, int key)
{
	bool first_round = true;
	int start_hash_index = hashtable_hashcode(table, key);
	int hash_index = start_hash_index;

	hashtable_item* item;

	// move in array until an empty 
	while((item = table->items[hash_index]) != NULL)
	{
		// if we reach the start again, that means we didnt find the item
		if(!first_round && hash_index == start_hash_index) break;
		
		first_round = false;

		if(item->key == key) return item;

		++hash_index;

		// wrap around the table
		hash_index %= table->size;
	}

	return NULL;
}

void hashtable_put(hashtable* table, hashtable_item* item)
{
	debug_assert(table->count < table->size, "No space left in the hashtable to put item");

	int start_hash_index = hashtable_hashcode(table, item->key);
	int hash_index = start_hash_index;

	hashtable_item** spot;

	// find the next empty slot
	while(*(spot = table->items + hash_index) != NULL)
	{
		++hash_index;

		// wrap back to the start of the table
		hash_index %= table->size;
	}

	table->count++;

	*spot = item;
}

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