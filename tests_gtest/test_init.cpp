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
Test réalisant de multiples fois une initialisation
suivie d'une alloc max
Définir DEBUG à la compilation pour avoir une sortie un
peu plus verbeuse.
*/

//define number of repeat to perform
#define NB_TESTS 10

//make one pass
static void alloc_max(size_t estimate, int pass) {
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
        EXPECT_EQ(estimate, last) << "Pass : " << pass;
    } else {
        last = estimate;
    }
}

// try several time the test to check idempotence
TEST(init, reapeated_alloc_max)
{
    for (int i = 0; i < NB_TESTS; i++) {
        mem_init();
        alloc_max(mem_space_get_size(), i);
    }
}
