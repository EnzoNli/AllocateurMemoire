//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

/*
Test réalisant des series d'allocations / désallocations
en ordre LIFO
Définir DEBUG à la compilation pour avoir une sortie un
peu plus verbeuse.
*/

//google test header
#include <gtest/gtest.h>

//alloc header
extern "C" {
    #include "../headers/mem_space.h"
    #include "../headers/mem.h"
    #include "../headers/mem_os.h"
}

using namespace testing;

#define MAX_ALLOC (1 << 10)
#define NB_TESTS 10

static void *allocs[MAX_ALLOC];

static int make_test(bool do_bzero) {
    int nb_alloc = 0;
    int i = 0;
    // On remplit la mémoire de blocs de taille croissante
    debug("Issuing a sequence of size increasing mallocs, starting from 0\n");
    while ((i < MAX_ALLOC) && ((allocs[i] = mem_alloc(i)) != NULL)) {
        if (do_bzero)
            bzero(allocs[i], i);
        i++;
    }
    i--;
    debug("Alloced up to %d bytes at %p\n", i, allocs[i]);
    nb_alloc = i;
    // On vide
    debug("Freeing all allocated memory\n");
    while (i >= 0) {
        mem_free(allocs[i]);
        // debug("Freed %p\n", allocs[i]);
        i--;
    }
    return nb_alloc;
}

TEST(base, no_bzero)
{
    bool do_bzero = false;
    int nb_alloc;
    nb_alloc = 0;
    mem_init();
    nb_alloc = make_test(do_bzero);
    for (int i = 0; i < NB_TESTS; i++) {
        // Teste si non idempotent !
        ASSERT_EQ(make_test(do_bzero), nb_alloc) << "Iteration " << i;
    }
}

TEST(base, bzero)
{
    bool do_bzero = true;
    int nb_alloc;
    nb_alloc = 0;
    mem_init();
    nb_alloc = make_test(do_bzero);
    for (int i = 0; i < NB_TESTS; i++) {
        // Teste si non idempotent !
        ASSERT_EQ(make_test(do_bzero), nb_alloc) << "Iteration " << i;
    }
}
