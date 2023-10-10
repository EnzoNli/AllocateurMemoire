#include "mem_space.h"
#include "mem.h"
#include "mem_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    mem_init();
    void *ptr = mem_alloc(100);
    if (ptr != NULL)
    {
        printf("Allocation réussie.\n");
        strcpy(ptr, "Hello, World!");
        printf("Ancien contenu : %s\n", (char *)ptr);

        ptr = mem_realloc(ptr, 200);
        if (ptr != NULL)
        {
            printf("Réallocation réussie.\n");
            printf("Nouveau contenu : %s\n", (char *)ptr);
            mem_free(ptr);
            printf("Libération de la mémoire.\n");
        }
        else
        {
            printf("Échec de la réallocation.\n");
            return -1;
        }
    }
    else
    {
        printf("Échec de l'allocation.\n");
        return -1;
    }
    return 0;
}