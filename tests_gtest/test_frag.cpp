//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

/*
Test réalisant des series d'allocations / désallocations
afin d'obtenir une forte fragmentation de manière aléatoire
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

#define MAX_ALLOC 100000
#define MAX_BLOC 200
#define FREQ_FREE 3

static void *allocs[MAX_ALLOC];

void run_frag(void)
{
    srand(time(NULL));

    int i = 0;
    int size = (rand() % MAX_BLOC) + 1;
    int free = 0;
    // On alloue en boucle des bloc de tailles variable et aléatoire comprise
    // entre 1 et MAX_BLOC
    while ((i < MAX_ALLOC) && (allocs[i] = mem_alloc(size)) != NULL) {
        debug("%d -------------------------------\n", i);
        debug("Allocation en %d\n",
               (int)((char *)allocs[i] - (char *)mem_space_get_addr()));
        ASSERT_LT(allocs[i], (void *)((char *)mem_space_get_addr() + mem_space_get_size()))
            << "Allocation en " << (int)((char *)allocs[i] - (char *)mem_space_get_addr());

        // On libère à intervalle aléatoire un bloc occupé d'adresse aléatoire
        // parmis les blocs alloué en mémoire
        if (rand() % FREQ_FREE == 0) {
            free = ((rand() % (i + 1)) - 1);
            debug("Libération %d\n", free);
            ASSERT_LT(allocs[free], (void *)((char *)mem_space_get_addr() + mem_space_get_size()));
            mem_free(allocs[free]);
            allocs[free] = NULL;
        }
        size = (rand() % MAX_BLOC) + 1;
        i++;
    }
    // Affichage à la fin du test une fois la mémoire trop fragmentée
    void * ptr_last = mem_alloc(size);
    ASSERT_EQ(ptr_last, nullptr) << "Le tableau d'allocation est trop petit, augmentez MAX_ALLOC ou MAX_BLOC\n";
    if (ptr_last == NULL) {
        printf("Tentative d'allocation de  %d octets.\n"
               "Impossible car la mémoire est trop fragmentée.\n"
               "%i blocs ont été alloué (certains ont peut-être été libérés)\n",
               size, i);
    }
}

TEST(frag, first_fit)
{
    mem_init();
    mem_set_fit_handler(mem_first_fit);
    run_frag();
}

TEST(frag, worst_fit)
{
    mem_init();
    mem_set_fit_handler(mem_worst_fit);
    run_frag();
}

TEST(frag, best_fit)
{
    mem_init();
    mem_set_fit_handler(mem_best_fit);
    run_frag();
}
