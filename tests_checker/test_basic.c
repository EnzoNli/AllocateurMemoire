//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

#include <assert.h>
#include "mem_checker.h"
#include "mem_os.h"

bool test_init(mem_checker_t * checker)
{
    //start a test
    mem_checker_start_test(checker, "init");

    //expect a single free block
    mem_checker_expect_free_block(checker, 128000);

    //perform check
    return mem_checker_check(checker);
}

bool test_one_alloc(mem_checker_t * checker)
{
    //start a test
    mem_checker_start_test(checker, "single alloc");

    //make one alloc
    mem_checker_alloc(checker, 10);

    //expect a single free block
    mem_checker_expect_alloc_block(checker, 10);//TODO ADAPT SIZE
    mem_checker_expect_free_block(checker, 128000 - 10);//TODO ADAPT SIZE

    //perform check
    return mem_checker_check(checker);
}

bool test_one_alloc_no_size(mem_checker_t * checker)
{
    //start a test
    mem_checker_start_test(checker, "single alloc no size");

    //make one alloc
    mem_checker_alloc(checker, 10);

    //expect a single free block
    mem_checker_expect_alloc_block(checker, MEM_UNCHECKED);
    mem_checker_expect_free_block(checker, MEM_UNCHECKED);

    //perform check
    return mem_checker_check(checker);
}

int main(int argc, char *argv[]) {
    //init alloc
    mem_init();

    //init checker
    mem_checker_t checker;
    mem_checker_init(&checker);

    //call tests
    assert(test_init(&checker));
    assert(test_one_alloc(&checker));
    assert(test_one_alloc_no_size(&checker));

    //ok
    return EXIT_SUCCESS;
}
