//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "mem_space.h"
#include "mem.h"
#include "mem_os.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define MAX_ALLOC (1 << 10)
#define NB_TESTS 10

static void *allocs[MAX_ALLOC];

static int make_test() {
    int nb_alloc = 0;
    int i = 0;
    // On remplit la mémoire de blocs de taille croissante
    debug("Issuing a sequence of size increasing mallocs, starting from 0\n");
    while ((i < MAX_ALLOC) && ((allocs[i] = mem_alloc(i)) != NULL)) {
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

int main(int argc, char *argv[]) {
    int nb_alloc;
    nb_alloc = 0;
    mem_init();
    fprintf(stderr, "Test réalisant des series d'allocations / désallocations "
                    "en ordre LIFO\n"
                    "Définir DEBUG à la compilation pour avoir une sortie un "
                    "peu plus verbeuse."
                    "\n");
    nb_alloc = make_test();
    for (int i = 0; i < NB_TESTS; i++) {
        // Teste si non idempotent !
        assert(make_test() == nb_alloc);
    }

    // TEST OK
    return 0;
}
