
#include "dynstr.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

dynstr* dynstr_new(char* str, u16 str_len, u16 buf_len)
{
	// @Todo: better asert?
	assert(str_len >= 0 && buf_len >= 0);

	// @Factor: use our custom allocator?
	dynstr* mem = (dynstr*) malloc(sizeof(dynstr));
	char* raw = (char*) malloc(buf_len + 1);

	dynstr* dstr = (dynstr*) mem;
	dstr->raw = raw;
	dstr->len = str_len;
	dstr->buf_len = buf_len;

	if (str != 0)
	{
		strncpy(raw, str, str_len);
	}

	// add the nullterm
	*(raw + str_len) = 0;

	return dstr;
}

dynstr* dynstr_new(char* str)
{
	size_t len = strlen(str);
	assert(len <= UINT16_MAX);

	return dynstr_new(str, (u16) len, (u16) len);
}

dynstr* dynstr_new(u16 buf_len)
{
	return dynstr_new(0, 0, buf_len);
}

dynstr* dynstr_new()
{
	return dynstr_new(0, 0, 0);
}

void dynstr_free(dynstr* str)
{
	free(str);
}

// @Optimize: reallocate 2* last size
// @Factor: we could factor this into some "set length" method?
// The dynstr pointer will still be valid, but the raw pointer might be invalidated
void dynstr_append_str(dynstr* to, char* from, u16 from_len)
{
	assert(from != 0);

	if (from_len == 0)
	{
		return;
	}

	int old_raw_len = to->len;
	int new_raw_len = old_raw_len + from_len;

	if(to->buf_len < new_raw_len)
	{
		to->raw = (char*) realloc(to->raw, new_raw_len + 1);
		to->buf_len = new_raw_len;
	}

	to->len = new_raw_len;

	strncpy(to->raw + old_raw_len, from, from_len);

	*(to->raw + to->len) = 0;
}

void dynstr_append_str(dynstr* to, char* from)
{
	return dynstr_append_str(to, from, strlen(from));
}

void dynstr_append_int(dynstr* to, int from)
{
	// @Optimize: slow :(
	int length = snprintf(NULL, 0, "%d", from);
	char* str = (char*) malloc(length + 1);
	snprintf(str, length + 1, "%d", from);

	dynstr_append_str(to, str, length);
}

dynstr* dynstr_append(dynstr* to, dynstr* from)
{
	dynstr_append_str(to, from->raw, from->len);
	return to; 
}

dynstr* dynstr_clear(dynstr* dstr)
{
	dstr->len = 0;
	dstr->raw[0] = 0;

	return dstr;
}

dynstr* dynstr_set_len(dynstr* dstr, u16 buf_len)
{


	return dstr;
}