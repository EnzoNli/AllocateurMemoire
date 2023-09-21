//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "../headers/mem_space.h"
#include "../headers/mem.h"
#include "../headers/mem_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Pour éviter les problèmes de ré-entrance.
static __thread int gbl_in_lib = 0;

// Astuce : on ne peut pas appeler sans rien faire printf() dans notre allocateur
//          car ..... printf appel malloc() => ré-entrance infinie.
#define dprintf(args...)                                                       \
    do {                                                                       \
        if (!gbl_in_lib) {                                                     \
            gbl_in_lib = 1;                                                    \
            printf(args);                                                      \
            gbl_in_lib = 0;                                                    \
        }                                                                      \
    } while (0)

// get min
#define min(a, b) ((a) < (b) ? (a) : (b))

// Initialise l'allocateur au premier usage
static void init() {
    // Pour se rappeler si déjà initialisé.
    static int first_use = 1;

    // au premier usage on appel mem_init
    if (first_use) {
        mem_init();
        first_use = 0;
    }
}

// Ecrase le symbol malloc() de la libc pour utiliser notre allocateur
void *malloc(size_t s) {
    // lazy init
    init();

    // debug
    dprintf("Allocation de %lu octets...", (unsigned long)s);

    // forward à notre allocateur
    void * result = mem_alloc(s);

    // debug
    if (!result)
        dprintf(" Alloc FAILED !!");
    else
        dprintf(" %lx\n", (unsigned long)result);

    // ok
    return result;
}

// Ecrase le symbol calloc() de la libc pour utiliser notre allocateur
void *calloc(size_t count, size_t size) {
    // calcule de la taille à allouer
    size_t total_size = count * size;

    // lazy init
    init();

    // debug
    dprintf("Allocation de %zu octets\n", total_size);

    // forward vers notre allocateur
    char * ptr = mem_alloc(total_size);

    // debug
    if (!ptr)
        dprintf(" Alloc FAILED !!");

    // gestion de la remise à zero
    if (ptr)
        memset(ptr, 0, total_size);

    //ok
    return ptr;
}

// Ecrase le symbol realloc() de la libc pour utiliser notre allocateur
void *realloc(void *ptr, size_t size) {
    char *result;

    // lazy init
    init();

    // debug
    dprintf("Reallocation de la zone en %lx\n", (unsigned long)ptr);

    // nouvelle allocation si rien avant
    if (ptr != NULL) {
        dprintf(" Realloc of NULL pointer\n");
        return mem_alloc(size);
    }

    // on récupère la taille du segment pour savoir quoi faire
    if (mem_get_size(ptr) >= size) {
        dprintf(" Useless realloc\n");
        return ptr;
    }

    // on alloue un nouveau segment
    result = mem_alloc(size);
    if (!result) {
        dprintf(" Realloc FAILED\n");
        return NULL;
    }

    // on calcule la taille à copier
    size_t copy_size = min(size, mem_get_size(ptr));
    memcpy(result, ptr, copy_size);

    // on libère l'ancien
    mem_free(ptr);

    // debug
    dprintf(" Realloc ok\n");

    // ok
    return result;
}

// Ecrase le symbol free() de la libc pour utiliser notre allocateur
void free(void *ptr) {
    // lazy init
    init();

    // si on a quelque chose ou rien
    if (ptr != NULL) {
        dprintf("Liberation de la zone en %lx\n", (unsigned long)ptr);
        mem_free(ptr);
    } else {
        dprintf("Liberation de la zone NULL\n");
    }
}
