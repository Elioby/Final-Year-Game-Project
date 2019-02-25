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
template<typename ... Args>
dynstr* dynstr_append(dynstr* to, char* format, Args ... strs)
{
	u32 args_len = sizeof...(Args);

	va_list args;
	va_start(args, format);

	u32 arg_index = 0;

	bool last_was_percent = false;
	for(u32 i = 0; true; i++)
	{
		char c = format[i];
		if(c == 0) break;

		if(last_was_percent)
		{
			if(c == 's')
			{
				dynstr_append_str(to, va_arg(args, char*));
			}
			else if(c == 'i')
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

		if(c != '%')
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

	va_end(args);

	return to;
}

dynstr* dynstr_append(dynstr* to, dynstr* from);
dynstr* dynstr_clear(dynstr* dstr);