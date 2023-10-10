#include "mem_space.h"
#include "mem.h"
#include "mem_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main()
{
    void *ptr1, *ptr2, *ptr3;

    mem_init();
    mem_set_fit_handler(&mem_first_fit);
    assert((ptr1 = mem_alloc(100)) != NULL);
    mem_set_fit_handler(&mem_best_fit);
    assert((ptr2 = mem_alloc(100)) != NULL);
    mem_set_fit_handler(&mem_worst_fit);
    assert((ptr3 = mem_alloc(100)) != NULL);

    printf("Allocations réussies avec différentes stratégies.\n");
    mem_free(ptr1);
    mem_free(ptr2);
    mem_free(ptr3);
    printf("Libérations de la mémoire.\n");
    return 0;
}
