//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

#ifndef MEM_CHECKER_H
#define MEM_CHECKER_H

//#define HAS_MEM_SIM

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#ifdef HAS_MEM_SIM
	#include "mem_sim.h"
#endif

typedef void (*mem_checker_test_start_handler_t)(void * arg, const char * name);
typedef void (*mem_checker_test_end_handler_t)(void * arg, bool status);
typedef void (*mem_checker_print_handler_t)(void * arg, const char * message);

#define MEM_UNCHECKED __SIZE_MAX__

/** Type of blocks to check. **/
typedef enum
{
	BLOCK_UNDEFINED,
	BLOCK_BUSY,
	BLOCK_FREED,
	BLOCK_STOP,
} mem_checker_block_type_t;

/**
Define an expected or active block to perform the check
and express what we expect.
**/
typedef struct
{
	size_t offset;
	size_t size;
	mem_checker_block_type_t type;
} mem_checker_block_t;

/**
Structure used to track the active allocations (memory ranges returned by mem_alloc)
and detect collisions and perform canary checking.
**/
typedef struct
{
	void * ptr;
	size_t size;
	char canary;
} mem_checker_alloc_t;

/** Handle dynamic resizable list of current blocs */
typedef struct
{
	/** List of current blocks returned by mem_show() **/
	mem_checker_block_t * blocks;
	/** Number of current blocks returned by mem_show() **/
	size_t count;
	/** Number of current blocks allocated to know if we need to reallocate to grow. **/
	size_t allocated;
} mem_checker_block_list_t;

/** State to track during the checking operation */
typedef struct
{
	/** Variable used by mem_checker_expect_block() to compute the offset of the next block to add. **/
	size_t expected_next_offset;
	/** Cursor to progress in current/expected while looping in mem_show() **/
	size_t check_cursor;
	/** Number of encoutered error, >1 indicate a failure of the check. **/
	size_t error_count;
	/** Has seen STOP **/
	bool seen_stop;
	/** If first test for callbacks **/
	bool is_first_test;
} mem_checker_checking_t;

/** Fields required to track the active allocations and detect collisions. **/
typedef struct
{
	/** List the allocated blocs for collide checking and canary checking. **/
	mem_checker_alloc_t * allocs;
	/** Current number of allocated blocks **/
	size_t count;
	/** Size of the allocated array for allocs to know if we need to relalloc to grow. **/
	size_t allocated;
} mem_checker_allocs_t;

/** Config of the checker **/
typedef struct
{
	/** Enable of disable the collide checking. **/
	bool check_collide;
	/** Check data corruption with canaries. **/
	bool check_corrupt;
	/** Pointer to the print function to be used to render the error messages. **/
	void (*print)(void * arg, const char * message);
	/** Argument to transmit when calling print. **/
	void * print_arg;
	/** Callback to call on entering test. **/
	void (*on_test_start)(void * arg, const char * name);
	/** Callback to call on terminating test. **/
	void (*on_test_end)(void * arg, bool status);
	/** Argument to transmit to on_* callbacks **/
	void * on_test_arg;
	/** enable simu **/
	bool enable_simu;
} mem_checker_config_t;

/** Struct to be used to generate the running summary. **/
typedef struct
{
	/** Name of the current test **/
	char * current_test_name;
	/** Log of the current test in case of error. **/
	char * current_test_log;
	bool quiet;
	bool display_err;
} mem_checker_test_summary_t;

/**
Object representation of the memory checker.
**/
typedef struct
{
	/** Track the current blocks returned by mem_show() **/
	mem_checker_block_list_t current;
	/** List the expected block to compare to the current one. **/
	mem_checker_block_list_t expected;
	/** State for the checking operations. **/
	mem_checker_checking_t checking;
	/** State to track the allocated chunks to detect collisions. **/
	mem_checker_allocs_t allocs;
	/** Configuration **/
	mem_checker_config_t config;
	#ifdef HAS_MEM_SIM
		/** simulator **/
		mem_sim_t simu;
	#endif
} mem_checker_t;

///////////////////////////// public functions
/**
Initilize the checker.
Remark, your also need to call mem_init() by yourself.
**/
void mem_checker_init(mem_checker_t * checker);
/** Finalize the checker to free its memory. **/
void mem_checker_fini(mem_checker_t * checker);
/** Start a test with a given name. It reset the allocator with mem_init() and all the checker internale states. **/
void mem_checker_start_test(mem_checker_t * checker, const char * test_name);
/** Load the commands from a file. **/
void mem_checker_load_script(mem_checker_t * checker, const char * filename);
/** Parse the given text command. **/
bool mem_checker_parse_command(mem_checker_t * checker, const char * command);
/** Perform the checking to validate we have the expected blocks by comparing the expect array with what returns mem_show(). **/
bool mem_checker_check(mem_checker_t * checker);
/** Reset the checking state. **/
void mem_checker_reset(mem_checker_t * checker, bool allocator);
/** Add an expected free block. The offset is computed from the end of the previous added block. **/
void mem_checker_expect_free_block(mem_checker_t * checker, size_t size);
/** Add an expected alloc block. The offset is computed from the end of the previous added block. **/
void mem_checker_expect_alloc_block(mem_checker_t * checker, size_t size);
/** Add an expected block. The offset is computed from the end of the previous added block. **/
void mem_checker_expect_block(mem_checker_t * checker, size_t size, bool is_free);
/** Add a stop block not the check what is after this position. **/
void mem_checker_ignore_next_blocks(mem_checker_t * checker);
/** Add an expected skip block which will just be ignored. The offset is computed from the end of the previous added block. **/
void mem_checker_expect_skiped_block(mem_checker_t * checker, size_t size);
void mem_checker_add_current_block(mem_checker_t * checker, size_t offset, size_t size, bool is_free);
/** Call mem_alloc() and check there is no collisions with the previously allocated blocks. it also fill the block with canary for corruption checking. **/
size_t mem_checker_alloc(mem_checker_t * checker, size_t alloc_size);
/** Call mem_free() but after cheking the canary added by mem_checker_alloc() to detect data corruption. **/
void mem_checker_free(mem_checker_t * checker, size_t alloc_id);
/** Enable of disable collide checking. **/
void mem_checker_set_collide_check(mem_checker_t * checker, bool collide_check);
/** Define the print function to know where to send the error messages. **/
void mem_checker_set_print(mem_checker_t * checker, mem_checker_print_handler_t print, void * arg);
/** Define the handlers to call on test start/end for pretty printing. **/
void mem_checker_set_test_handler(mem_checker_t * checker, mem_checker_test_start_handler_t on_test_start,
	mem_checker_test_end_handler_t on_test_end, void * arg);
bool mem_checker_sim_config_set(mem_checker_t * checker, const char * varname, const char * value);
void mem_checker_sim_enable(mem_checker_t * checker);
void mem_checker_expect_from_simu(mem_checker_t * checker);
void mem_checker_sim_show(mem_checker_t * checker);
void mem_checker_mem_show(mem_checker_t * checker);
bool mem_checker_check_sim(mem_checker_t * checker);

////////////////////////// special
/** Run by reading the commands from the given file descriptor to execute the given script. fp can be stdin to get commands from the terminal. **/
bool mem_checker_run(FILE * fp, bool stop_on_first, bool quiet, bool display_err, const char * fname);

//private functions
//static void mem_checker_load_mem_show(mem_checker_t * checker, bool check);
//static bool mem_checker_compare_block(mem_checker_block_t * a, mem_checker_block_t * b);
//static void mem_checker_print_block(const char * prefix, mem_checker_block_t * block, const char* color);
//static bool mem_checker_alloc_collide(mem_checker_t * checker, size_t id1, size_t id2);

#endif //MEM_CHECKER_H
