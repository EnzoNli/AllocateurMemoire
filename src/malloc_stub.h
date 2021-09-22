#ifndef __MALLOC_STUB_H__
#define __MALLOC_STUB_H__
#include <stdlib.h>
void *malloc(size_t s);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);
#endif
