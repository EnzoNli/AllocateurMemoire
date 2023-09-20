//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_ALLOC 100000
#define MAX_BLOC 200
#define FREQ_FREE 3

static void *allocs[MAX_ALLOC];

int main(int argc, char *argv[]) {
    mem_init();
    if (argc == 2) {
        if (strcmp(argv[1], "first") == 0) {
            mem_set_fit_handler(mem_first_fit);
            printf("Stratégie first fit\n");
        } else if (strcmp(argv[1], "best") == 0) {
            mem_set_fit_handler(mem_best_fit);
            printf("Stratégie best fit\n");
        } else if (strcmp(argv[1], "worst") == 0) {
            mem_set_fit_handler(mem_worst_fit);
            printf("Stratégie worst fit\n");
        }
    }
    srand(time(NULL));
    fprintf(stderr,
            "Test réalisant des series d'allocations / désallocations\n"
            "afin d'obtenir une forte fragmentation de manière aléatoire\n");

    int i = 0;
    int size = (rand() % MAX_BLOC) + 1;
    int free = 0;
    // On alloue en boucle des bloc de tailles variable et aléatoire comprise
    // entre 1 et MAX_BLOC
    while ((i < MAX_ALLOC) && (allocs[i] = mem_alloc(size)) != NULL) {
        printf("%d -------------------------------\n", i);
        printf("Allocation en %d\n",
               (int)((char *)allocs[i] - (char *)mem_space_get_addr()));
        assert(allocs[i] <
               (void *)((char *)mem_space_get_addr() + mem_space_get_size()));

        // On libère à intervalle aléatoire un bloc occupé d'adresse aléatoire
        // parmis les blocs alloué en mémoire
        if (rand() % FREQ_FREE == 0) {
            free = ((rand() % (i + 1)) - 1);
            printf("Libération %d\n", free);
            assert(allocs[free] <
                   (void *)((char *)mem_space_get_addr() + mem_space_get_size()));
            mem_free(allocs[free]);
            allocs[free] = NULL;
        }
        size = (rand() % MAX_BLOC) + 1;
        i++;
    }
    // Affichage à la fin du test une fois la mémoire trop fragmentée
    if (mem_alloc(size) == NULL) {
        printf("Tentative d'allocation de  %d octets.\n"
               "Impossible car la mémoire est trop fragmentée.\n"
               "%i blocs ont été alloué (certains ont peut-être été libérés)\n",
               size, i);
    } else {
        printf("Le tableau d'allocation est trop petit, augmentez MAX_ALLOC ou "
               "MAX_BLOC\n");
    }

    // Fin du test
    return 0;
}
