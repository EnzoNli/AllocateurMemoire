#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>

static __thread int in_lib=0;

#define dprintf(args...) \
  do { \
    if (!in_lib) { \
      in_lib=1; \
      printf(args); \
      in_lib=0; \
    } \
  } while (0)

static
void init() {
    static int first=1;

    if (first) {
        mem_init();
        first = 0;
    }
}

void *malloc(size_t s) {
    void *result;

    init();
    dprintf("Allocation de %lu octets...", (unsigned long) s);
    result = mem_alloc(s);
    if (!result)
        dprintf(" Alloc FAILED !!");
    else
	dprintf(" %lx\n", (unsigned long) result);
    return result;
}

void *calloc(size_t count, size_t size) {
    int i;
    char *p;
    size_t s = count*size;

    init();
    dprintf("Allocation de %zu octets\n", s);
    p = mem_alloc(s);
    if (!p)
        dprintf(" Alloc FAILED !!");
    if (p)
        for (i=0; i<s; i++)
            p[i] = 0;
    return p;
}

void *realloc(void *ptr, size_t size) {
    size_t s;
    char *result;

    init();
    dprintf("Reallocation de la zone en %lx\n", (unsigned long) ptr);
    if (!ptr) {
        dprintf(" Realloc of NULL pointer\n");
        return mem_alloc(size);
    }
    if (mem_get_size(ptr) >= size) {
        dprintf(" Useless realloc\n");
        return ptr;
    }
    result = mem_alloc(size);
    if (!result) {
        dprintf(" Realloc FAILED\n");
        return NULL;
    }
    for (s = 0; s<mem_get_size(ptr); s++)
        result[s] = ((char *) ptr)[s];
    mem_free(ptr);
    dprintf(" Realloc ok\n");
    return result;
}

void free(void *ptr) {
    init();
    if (ptr) {
        dprintf("Liberation de la zone en %lx\n", (unsigned long) ptr);
        mem_free(ptr);
    } else {
        dprintf("Liberation de la zone NULL\n");
    }
}
