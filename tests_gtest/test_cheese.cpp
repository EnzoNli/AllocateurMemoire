//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

/*
Test réalisant récursivement une allocation en gruyère
selon le modèle d'appel de fibonacci.
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

#define NB_TESTS 5
#define NB_MAX_STORES 100

static bool gbl_use_bzero = false;

void my_free(void **mem) {
    if (*mem != NULL) {
        mem_free(*mem);
        // debug("Freed %p\n", *mem);
        *mem = NULL;
    }
}

static void *checked_alloc(size_t s) {
    void *result;

    assert((result = mem_alloc(s)) != NULL);
    if (gbl_use_bzero)
        bzero(result, s);
    // debug("Alloced %zu bytes at %p\n", s, result);
    return result;
}

int first = 1;
int nb_allocs = 0;
void *allocs[NB_MAX_STORES];

void reset() {
    first = 0;
    nb_allocs = 0;
}

void store_or_check(void *adr) {
    if (nb_allocs < NB_MAX_STORES) {
        if (first)
            allocs[nb_allocs++] = adr;
        else
            assert(allocs[nb_allocs++] == adr);
    }
}

void alloc_fun(int n) {
    void *a, *b, *c;
    if (n < 0)
        return;
    a = checked_alloc(5);
    store_or_check(a);
    b = checked_alloc(10);
    store_or_check(b);
    alloc_fun(n - 1);
    my_free(&a);
    c = checked_alloc(5);
    store_or_check(c);
    alloc_fun(n - 2);
    my_free(&c);
    my_free(&b);
}

TEST(cheese, no_bzero)
{
    mem_init();
    gbl_use_bzero = false;
    for (int i = 0; i < NB_TESTS; i++) {
        debug("Issuing test number %d\n", i);
        alloc_fun(6);
        reset();
    }
}

TEST(cheese, bzero)
{
    mem_init();
    gbl_use_bzero = true;
    for (int i = 0; i < NB_TESTS; i++) {
        debug("Issuing test number %d\n", i);
        alloc_fun(6);
        reset();
    }
}
