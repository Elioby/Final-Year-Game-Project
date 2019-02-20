#include "file.h"

#include <stdio.h>

file_data* load_file(char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "r");

	if(!file) return NULL;

	fseek(file, 0, SEEK_END);
	u32 file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	// pack the struct and file data into one memory allocation
	file_data* data = (file_data*)malloc(sizeof(file_data) + file_size);

	// the data pointer is after the struct data
	void* file_buffer = (void*)((char*)data + sizeof(file_data));
	data->length = file_size;
	data->data = file_buffer;

	fread_s(file_buffer, file_size, file_size, 1, file);

	fclose(file);

	return data;
}