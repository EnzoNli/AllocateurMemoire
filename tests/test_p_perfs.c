#include "mem_space.h"
#include "mem.h"
#include "mem_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_ALLOC (1 << 10)

static void *allocs[MAX_ALLOC];

static int frag_first_fit()
{
    int nb_alloc = 0;

    assert((allocs[nb_alloc++] = mem_alloc(20000)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(30000)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(15000)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(40950)) != NULL);

    mem_free(allocs[2]);

    assert((allocs[nb_alloc++] = mem_alloc(16000)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(16000)) == NULL);

    mem_free(allocs[0]);
    assert((allocs[nb_alloc++] = mem_alloc(20500)) == NULL);

    return nb_alloc;
}

static int frag_best_fit()
{
    int nb_alloc = 0;

    assert((allocs[nb_alloc++] = mem_alloc(40000)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(20000)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(12000)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(36000)) != NULL);

    mem_free(allocs[2]);

    assert((allocs[nb_alloc++] = mem_alloc(18500)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(11450)) != NULL);

    mem_free(allocs[0]);
    assert((allocs[nb_alloc++] = mem_alloc(39500)) != NULL);

    assert((allocs[nb_alloc++] = mem_alloc(2000)) == NULL);

    return nb_alloc;
}

static int frag_worst_fit()
{
    int nb_alloc = 0;

    assert((allocs[nb_alloc++] = mem_alloc(70000)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(30000)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(20000)) != NULL);

    mem_free(allocs[1]);

    assert((allocs[nb_alloc++] = mem_alloc(16)) != NULL);
    assert((allocs[nb_alloc++] = mem_alloc(6000)) != NULL);

    mem_free(allocs[3]);
    assert((allocs[nb_alloc++] = mem_alloc(23000)) != NULL);

    assert((allocs[nb_alloc++] = mem_alloc(8000)) == NULL);

    return nb_alloc;
}

static void free_all(int nb_allocs)
{
    for (size_t i = 0; i < nb_allocs; i++)
    {
        mem_free(allocs[i]);
    }
}

static void run_test(int strategie)
{
    int nb_allocs = 0;
    switch (strategie)
    {
    case 0:
        mem_init();
        printf("Testing First Fit allocation strategy\n");
        printf("--------------------------------\n");
        printf("Frag first fit checking...\n\n");
        nb_allocs = frag_first_fit();

        printf("Freeing all allocs.\n");
        free_all(nb_allocs);
        printf("Frag first fit checked.\n\n");
        break;
    case 1:
        mem_init();
        mem_set_fit_handler(&mem_best_fit);
        printf("Testing Best Fit allocation strategy\n");
        printf("--------------------------------\n");
        printf("Frag best fit checking...\n\n");
        nb_allocs = frag_best_fit();

        printf("Freeing all allocs.\n");
        free_all(nb_allocs);
        printf("Frag best fit checked.\n\n");
        break;

    case 2:
        mem_init();
        mem_set_fit_handler(&mem_worst_fit);
        printf("Testing Worst Fit allocation strategy\n");
        printf("--------------------------------\n");
        printf("Frag worst fit checking...\n\n");
        nb_allocs = frag_worst_fit();

        printf("Freeing all allocs.\n");
        free_all(nb_allocs);
        printf("Frag worst fit checked.\n\n");
        break;
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    run_test(0);
    printf("\n");

    run_test(1);
    printf("\n");

    run_test(2);
    return 0;
}
