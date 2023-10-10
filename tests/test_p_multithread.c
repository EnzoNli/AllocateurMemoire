#include "mem_space.h"
#include "mem.h"
#include "mem_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void *alloc_and_free()
{
    void *ptr = mem_alloc(100);
    mem_free(ptr);
    return NULL;
}

int main()
{
    mem_init();
    pthread_t threads[10];
    for (int i = 0; i < 10; ++i)
    {
        pthread_create(&threads[i], NULL, alloc_and_free, NULL);
    }
    for (int i = 0; i < 10; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    printf("Tests multithread terminés.\n");
    return 0;
}
