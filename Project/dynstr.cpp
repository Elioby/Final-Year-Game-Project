
#include "dynstr.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

dynstr* dynstr_new(char* str, u16 str_len, u16 buf_len)
{
	// @Todo: better asert?
	assert(str_len >= 0 && buf_len >= 0 && "You can't have a negatively sized string or buffer");

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
	assert(len <= UINT16_MAX && "Dynstr only supports strings of length <= 65535");

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
	free(str->raw);
}

dynstr* dynstr_append_va(dynstr* to, char* format, va_list args)
{
	u32 arg_index = 0;

	bool last_was_percent = false;
	for (u32 i = 0; true; i++)
	{
		char c = format[i];
		if (c == 0) break;

		if (last_was_percent)
		{
			if (c == 's')
			{
				dynstr_append_str(to, va_arg(args, char*));
			}
			else if (c == 'i')
			{
				int integer = va_arg(args, int);
				dynstr_append_int(to, integer);
			}
			else if (c == '%')
			{
				// @Optimize: use a single char append method?
				dynstr_append_str(to, "%", 1);
			}

			last_was_percent = false;
			continue;
		}

		if (c != '%')
		{
			char ch[2];
			ch[0] = c;
			ch[1] = 0;

			// @Optimize: use a single char append method?
			dynstr_append_str(to, ch, 1);
		}
		else
		{
			last_was_percent = true;
		}
	}
}

dynstr* dynstr_append(dynstr* to, char* format, ...)
{
	va_list args;
	va_start(args, format);

	dynstr_append_va(to, format, args);

	va_end(args);

	return to;
}

// @Optimize: reallocate 2* last size
// @Factor: we could factor this into some "set length" method?
// The dynstr pointer will still be valid, but the raw pointer might be invalidated
void dynstr_append_str(dynstr* to, char* from, u16 from_len)
{
	assert(from != 0 && "Cannot append null string");
	assert((u32) from_len + (u32) to->len < UINT16_MAX && "Dynstr only supports strings of length <= 65535");

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
	size_t from_len = strlen(from);
	assert(from_len < UINT16_MAX && "Dynstr only supports strings of length <= 65535");

	return dynstr_append_str(to, from, (u16) from_len);
}

void dynstr_append_int(dynstr* to, int from)
{
	// @Optimize: slow :(
	int length = snprintf(NULL, 0, "%d", from);
	char* str = (char*) malloc(length + 1);
	snprintf(str, length + 1, "%d", from);

	dynstr_append_str(to, str, length);
	free(str);
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