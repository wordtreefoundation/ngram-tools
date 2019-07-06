#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#define true 1
#define false 0

extern int VERBOSE;

FILE* open_file(const char* path);
void print_range(char* start_ptr, size_t len);

#endif
