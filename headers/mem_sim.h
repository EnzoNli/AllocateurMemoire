//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

#ifndef MEM_SIM_H
#define MEM_SIM_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef MEMORY_SIZE
    #define MEMORY_SIZE 128000
#endif

typedef enum {
    FIRST_FIT,
    BEST_FIT,
    WORST_FIT
} mem_sim_strategy_t;

typedef enum {
    FREE_ALLOC,
    ALLOC_FREE,
} mem_sim_split_strategy_t;

typedef struct
{
    size_t memory;
    size_t busy_block_size;
    size_t free_block_size;
    size_t header_size;
    size_t align_size;       //default value is no alignement
    size_t ptr_size;         //default value = 4 bytes = 32bit architecture
    mem_sim_strategy_t strategy;
    mem_sim_split_strategy_t keep_strategy;
} mem_sim_config_t;

typedef struct
{
    size_t align_header;
    size_t align_busy_block;
    size_t align_free_block;
    size_t min_free_zone;
} mem_sim_config_computed_t;

typedef enum {
    FREE,
    BUSY
} mem_sim_state_t;

typedef struct mem_sim_block_s {
    mem_sim_state_t block_state;  //busy or free
    size_t asked;                  //if busy, the size given to malloc
    size_t allocated;              //if busy, the real allocated size, depends on the alignement
    size_t control;                //the size of the control structure, depends on busy or free, aligned or not
    size_t total;                  //the total size of the block
    size_t start_address;          //start address of the block
    size_t user_address;           //if busy, address of the accessible (allocated) zone
    struct mem_sim_block_s* next; //point to nexy block
} mem_sim_block_t;

/** Define the mem simulator main struct. **/
typedef struct 
{
    mem_sim_config_t config;
    mem_sim_config_computed_t computed;
    mem_sim_block_t * mem_state;
    size_t nb_blocks;
} mem_sim_t;

typedef void (*mem_sim_on_block_handler)(mem_sim_block_t * block, void * arg);

/** Initilize the simulator. **/
void mem_sim_init(mem_sim_t * simu, size_t memor_size);
/** Set the variable with given name to the given value. **/
bool mem_sim_config_set(mem_sim_t * simu, const char * varname, const char * value);
/** Copy the variable value as string in outvalue **/
bool mem_sim_config_get(mem_sim_t * simu, const char * varname, char * outvalue, size_t maxsize);
/** Start the simulator **/
void mem_sim_start(mem_sim_t * simu);
/** Simulate an allocation and compute the expected address **/
void * mem_sim_alloc(mem_sim_t * simu, size_t size);
/** Simulate a free **/
void mem_sim_free(mem_sim_t * simu, void * ptr);
/** Loop on all blocks in the simuated allocator **/
void mem_sim_for_each_block(mem_sim_t * simu, mem_sim_on_block_handler handler, void *arg);
/** Display the state of the allocator. **/
void mem_sim_show(mem_sim_t * simu);
/** Display the configuration. **/
void mem_sim_show_config(mem_sim_t * simu);
/** Reset the simulation. **/
void mem_sim_reset(mem_sim_t * simu);
/** Clenaup the memory allocated by the simulator. **/
void mem_sim_fini(mem_sim_t * simu);

#endif //MEM_SIM_H
