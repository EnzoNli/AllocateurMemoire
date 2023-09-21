//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

//include google test header
#include <gtest/gtest.h>

//include alloc definitions from C language into current file (c++)
extern "C" {
    #include "../headers/mem_os.h"
    #include "../headers/mem_space.h"
    #include "../headers/mem.h"
}

//use the testing namespace to get google test functions
using namespace testing;

//basic test example 1
TEST(basic, alloc_free_recycle)
{
    mem_init();

    //allocate first
    void * ptr1 = mem_alloc(32);
    EXPECT_NE(nullptr, ptr1);

    //free it
    mem_free(ptr1);

    //alloc again
    void * ptr2 = mem_alloc(32);
    EXPECT_EQ(ptr2, ptr1) << "Didn't reallocated the same bloc !";
}

// you can add you own tests here
