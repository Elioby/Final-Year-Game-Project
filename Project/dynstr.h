#pragma once

#include "general.h"

struct dynstr
{
	char* raw;

	// @Todo: dynamically sized int len for different sized strings? (to save memory, and for longer strings than 16bit uint)
	u16 len;

	// @Todo: hide this?
	u16 buf_len;
};

dynstr* dynstr_create(char* str);
dynstr* dynstr_create(char* str, u16 len);
dynstr* dynstr_create(u16 buf_len);

void dynstr_destroy(dynstr* str);

void dynstr_append_str(dynstr* to, char* from);
void dynstr_append_str(dynstr* to, char* from, u16 from_len);

void dynstr_append_int(dynstr* to, int from);

// takes a variable number of char* to append
dynstr* dynstr_append(dynstr* to, char* format, ...);
dynstr* dynstr_append_va(dynstr* to, char* format, va_list args);

dynstr* dynstr_append(dynstr* to, dynstr* from);
dynstr* dynstr_clear(dynstr* dstr);

// move the start of the string forward
dynstr* dynstr_trim_start(dynstr* dstr, u16 amount);