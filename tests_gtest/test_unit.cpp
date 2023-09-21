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
}

//use the testing namespace to get google test functions
using namespace testing;

//basic test example 1
TEST(unit, mem_space_get_addr)
{
    mem_init();
    EXPECT_NE(nullptr, mem_space_get_addr());
}

//basic test example 2
TEST(unit, mem_space_get_size)
{
    mem_init();
    EXPECT_NE(0lu, mem_space_get_size());
}

// you can add you own tests here
