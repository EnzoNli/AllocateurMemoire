#include "common.h"

// Vous pouvez changer la taille de la mémoire ici,
// si vous n'utilisez pas l'option -DMEMORY_SIZE=...
// lors de la compilation
#if !defined(MEMORY_SIZE)
#define MEMORY_SIZE 4096
#endif

static char memory[MEMORY_SIZE];

void *get_memory_adr() { return memory; }

size_t get_memory_size() { return MEMORY_SIZE; }
