#pragma once

#include "general.h"

struct dynstr
{
	char* raw;

	// @Todo: dynamically sized int len for different sized strings? (to save memory, and for longer strings than 16bit uint)
	u16 len;
	u16 buf_len;
};

dynstr* dynstr_new(char* str);
dynstr* dynstr_new(u16 buf_len);

void dynstr_free(dynstr* str);

void dynstr_append_str(dynstr* to, char* from);
void dynstr_append_str(dynstr* to, char* from, u16 from_len);

void dynstr_append_int(dynstr* to, int from);

// Takes a variable number of char* to append
dynstr* dynstr_append(dynstr* to, char* format, ...);
dynstr* dynstr_append_va(dynstr* to, char* format, va_list args);

dynstr* dynstr_append(dynstr* to, dynstr* from);
dynstr* dynstr_clear(dynstr* dstr);