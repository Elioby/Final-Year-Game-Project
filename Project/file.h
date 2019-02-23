#pragma once

#include "general.h"

struct file_data {
	void* data;
	u32 length;
};

file_data* load_file(char* filename);