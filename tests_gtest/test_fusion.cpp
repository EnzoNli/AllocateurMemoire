//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

//google test header
#include <gtest/gtest.h>

//alloc header
extern "C" {
    #include "../headers/mem_space.h"
    #include "../headers/mem.h"
    #include "../headers/mem_os.h"
}

using namespace testing;

/*
Test réalisant divers cas de fusion (avant, arrière et double
Définir DEBUG à la compilation pour avoir une sortie un peu plus
verbeuse.
*/

#define MAX_ALLOC (1 << 10)
#define NB_TESTS 5

// adresse par rapport au début de la mémoire
void *relative_adr(void *adr) {
    return (void *)((char *)adr - (char *)mem_space_get_addr());
}

void my_free(void **mem) {
    if (*mem != NULL) {
        mem_free(*mem);
        debug("Freed %p\n", relative_adr(*mem));
        *mem = NULL;
    }
}

static void *checked_alloc(size_t s) {
    void *result = mem_alloc(s);
    EXPECT_NE(nullptr, result) << "Fail to allocate memory !";
    EXPECT_GE(result, mem_space_get_addr()) << "Not in allocator space (before) !";
    EXPECT_LE(result, (void*)((size_t)mem_space_get_addr() + mem_space_get_size())) << "Not in allocator space (after) !";
    debug("Alloced %zu bytes at %p\n", s, relative_adr(result));
    return result;
}

//make one pass
static void * alloc_max(size_t estimate) {
    //vars
    void *result;
    static size_t last = 0;

    //allocate much memory as many as possible in on call
    //if fail, try less until success
    while ((result = mem_alloc(estimate)) == NULL) {
        estimate--;
    }

    //debug
    debug("Alloced %zu bytes at %p\n", estimate, result);

    //check that we allocated as many as the previous call
    if (last > 0) {
        // Idempotence test
        EXPECT_EQ(estimate, last);
    } else {
        last = estimate;
    }

    //ret
    return result;
}

static void alloc5(void **ptr) {
    ptr[0] = checked_alloc(MAX_ALLOC);
    ptr[1] = checked_alloc(MAX_ALLOC);
    ptr[2] = checked_alloc(MAX_ALLOC);
    ptr[3] = checked_alloc(MAX_ALLOC);
    ptr[4] = alloc_max(mem_space_get_size() - 4 * MAX_ALLOC);
}

static void free5(void **ptr) {
    for (int i = 0; i < 5; i++) {
        my_free(&ptr[i]);
    }
}

TEST(fusion, front_back_both)
{
    void *ptr[5];

    mem_init();
    for (int i = 0; i < NB_TESTS; i++) {
        debug("Pass %d\n", i);

        debug("Fusion avant\n");
        alloc5(ptr);
        my_free(&ptr[2]);
        my_free(&ptr[1]);
        ptr[1] = checked_alloc(2 * MAX_ALLOC);
        free5(ptr);
        ASSERT_FALSE(HasFailure());

        debug("Fusion arrière\n");
        alloc5(ptr);
        my_free(&ptr[1]);
        my_free(&ptr[2]);
        ptr[1] = checked_alloc(2 * MAX_ALLOC);
        free5(ptr);
        ASSERT_FALSE(HasFailure());

        debug("Fusion avant/arrière\n");
        alloc5(ptr);
        my_free(&ptr[1]);
        my_free(&ptr[3]);
        my_free(&ptr[2]);
        ptr[1] = checked_alloc(3 * MAX_ALLOC);
        free5(ptr);
        ASSERT_FALSE(HasFailure());
    }
}
