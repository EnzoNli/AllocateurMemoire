//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

//std C
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <string.h>
//internal
#include "mem_checker.h"
#include "mem.h"
#include "mem_os.h"
#include "mem_space.h"

static const char * cst_type_name[] = {"Unexpected", "Busy", "Free"};

#define COLOR_NONE NULL
#define COLOR_RED "\e[31m"
#define COLOR_GREEN "\e[32m"
#define COLOR_YELLOW "\e[33m"
#define COLOR_PURPLE "\e[35m"
#define COLOR_RESET "\e[0m"

#define min(a, b) (((a) > (b)) ? (b) : (a))

#define QUIET_PADDING 80

#define CHECKER_PRINT(checker, ...) \
    do { \
        char buffer[4096]; \
        if (checker->config.print == NULL) { \
            fprintf(stdout, __VA_ARGS__); \
        } else { \
            sprintf(buffer, __VA_ARGS__); \
            checker->config.print(checker->config.print_arg, buffer); \
        } \
    } while (0)

//list static functions not to depend on order
static void mem_checker_expect_block_internal(mem_checker_t * checker, size_t size, mem_checker_block_type_t type);
static size_t mem_checker_get_offset(void * addr);
static bool mem_checker_regexp_command(const char* command, const char * regexp, size_t * value, size_t * repeat);
static bool mem_checker_compare_block(mem_checker_block_t * expected, mem_checker_block_t * has);
static void mem_checker_print_block(mem_checker_t * checker, const char * prefix, mem_checker_block_t * block, const char* color);
static void mem_checker_mem_show_print(void * ptr, size_t size, int free);
static void mem_checker_mem_show_print_and_check(void * ptr, size_t size, int free);
static void mem_checker_load_mem_show(mem_checker_t * checker, bool check);
static void mem_checker_check_corrupt(mem_checker_t * checker, size_t alloc_id);
static void mem_checker_expect_block_internal(mem_checker_t * checker, size_t size, mem_checker_block_type_t type);
static bool mem_checker_alloc_collide(mem_checker_t * checker, size_t id1, size_t id2);
static void mem_checker_cut_on_line_break(char * value);
static void mem_checker_run_quiet_log(void * arg, const char * message);
static void mem_checker_run_quiet_start(void * arg, const char * name);
static void mem_checker_run_quiet_end(void * arg, bool status);
static bool mem_checker_check_next(mem_checker_t * checker);

static size_t mem_checker_get_offset(void * addr)
{
    if (addr == NULL)
        return 0;
    //return (size_t)addr;
    return addr - mem_space_get_addr();
}

static size_t mem_checker_get_print_offset(void * addr)
{
    if (addr == NULL)
        return 0;
    if ((size_t)addr < mem_space_get_size())
        return (size_t)addr;
    else
        return addr - mem_space_get_addr();
}

void mem_checker_init(mem_checker_t * checker)
{
    //check
    assert(checker != NULL);

    //default init size
    const size_t cnt_max_init = 4096;

    //alloc
    checker->current.blocks = calloc(cnt_max_init, sizeof(mem_checker_block_t));
    checker->current.count = 0;
    checker->current.allocated = cnt_max_init;

    //alloc
    checker->expected.blocks = calloc(cnt_max_init, sizeof(mem_checker_block_t));
    checker->expected.count = 0;
    checker->expected.allocated = cnt_max_init;

    //alloc
    checker->allocs.allocs =  calloc(cnt_max_init, sizeof(mem_checker_alloc_t));
    checker->allocs.count = 0;
    checker->allocs.allocated = cnt_max_init;

    //check cur
    checker->checking.check_cursor = 0;
    checker->checking.error_count = 0;
    checker->checking.is_first_test = true;

    //set next
    checker->checking.expected_next_offset = 0;

    //optional
    checker->config.check_collide = true;
    checker->config.check_corrupt = true;
    checker->config.enable_simu = false;

    //set print
    checker->config.print = NULL;
    checker->config.print_arg = NULL;
    checker->config.on_test_start = NULL;
    checker->config.on_test_end = NULL;
    checker->config.on_test_arg = NULL;

    //init simu
    #ifdef HAS_MEM_SIM
        mem_sim_init(&checker->simu, MEMORY_SIZE);
    #endif
}

void mem_checker_fini(mem_checker_t * checker)
{
    //check
    assert(checker != NULL);

    //error
    if (checker->expected.count != 0)
        CHECKER_PRINT(checker, COLOR_RED "ERROR: have non checked expected rules, you might forgot the 'Check' clause !" COLOR_RESET "\n");

    //dump last test
    if (checker->config.on_test_end)
        checker->config.on_test_end(checker->config.on_test_arg, (checker->checking.error_count == 0));

    //delete
    free(checker->current.blocks);
    free(checker->expected.blocks);
    free(checker->allocs.allocs);

    //simu
    #ifdef HAS_MEM_SIM
        mem_sim_fini(&checker->simu);
    #endif
}

void mem_checker_reset(mem_checker_t * checker, bool allocator)
{
    //check
    assert(checker != NULL);

    //reset
    checker->current.count = 0;
    checker->expected.count = 0;
    checker->checking.expected_next_offset = 0;
    checker->checking.check_cursor = 0;

    //reset allocator state
    if (allocator) {
        mem_init();
        checker->allocs.count = 0;
        #ifdef HAS_MEM_SIM
            mem_sim_reset(&checker->simu);
        #endif
    }
}

void mem_checker_start_test(mem_checker_t * checker, const char * test_name)
{
    //check
    assert(checker != NULL);
    assert(test_name != NULL);

    //error
    if (checker->expected.count != 0)
        CHECKER_PRINT(checker, COLOR_RED "ERROR: have expected rules not checked, you might forget the 'Check' clause !" COLOR_RESET "\n");

    //call
    if (checker->config.on_test_end != NULL && checker->checking.is_first_test == false)
        checker->config.on_test_end(checker->config.on_test_arg, (checker->checking.error_count == 0));
    if (checker->config.on_test_start != NULL)
        checker->config.on_test_start(checker->config.on_test_arg, test_name);

    //apply
    CHECKER_PRINT(checker, COLOR_YELLOW "================ %s ==============" COLOR_RESET "\n", test_name);
    mem_init();
    mem_checker_reset(checker, true);

    //set
    checker->checking.is_first_test = false;
}

#ifdef HAS_MEM_SIM
static bool mem_checker_regexp_sim_set(const char* command, const char * regexp, char * varname, char * value, size_t max_cnt)
{
    //check
    assert(command != NULL);
    assert(regexp != NULL);

    //build full rexexp
    char tmp_regexp[4096];
    sprintf(tmp_regexp, "^(%s) ([A-Za-z0-9_-]+) ([A-Za-z0-9_-]+)$", regexp);

    //compile
    regex_t compiled_regexp;
    int status = regcomp(&compiled_regexp, tmp_regexp, REG_EXTENDED);
    if (status != 0) {
        fprintf(stderr, "Failed to compile regexp : %s\n", tmp_regexp);
        abort();
    }

    //apply
    regmatch_t matches[4];
    status = regexec(&compiled_regexp, command, 4, matches,  0);

    //free regexp
    regfree(&compiled_regexp);

    //error
    if (status != 0)
        return false;

    //extract infos
    if (varname != NULL && matches[2].rm_so != -1) {
        size_t cnt = min(max_cnt, matches[2].rm_eo - matches[2].rm_so);
        strncpy(varname, command + matches[2].rm_so, cnt);
        varname[cnt] = '\0';
    }
    if (value != NULL && matches[3].rm_so != -1) {
        size_t cnt = min(max_cnt, matches[3].rm_eo - matches[3].rm_so);
        strncpy(value, command + matches[3].rm_so, cnt);
        value[cnt] = '\0';
    }

    //return status
    return status == 0;
}
#endif

static bool mem_checker_regexp_command(const char* command, const char * regexp, size_t * value, size_t * repeat)
{
    //check
    assert(command != NULL);
    assert(regexp != NULL);

    //build full rexexp
    char tmp_regexp[4096];
    sprintf(tmp_regexp, "^(%s)( [0-9]+)?( [0-9]+x)?$", regexp);

    //compile
    regex_t compiled_regexp;
    int status = regcomp(&compiled_regexp, tmp_regexp, REG_EXTENDED);
    if (status != 0) {
        fprintf(stderr, "Failed to compile regexp : %s\n", tmp_regexp);
        abort();
    }

    //apply
    regmatch_t matches[4];
    status = regexec(&compiled_regexp, command, 4, matches,  0);

    //free regexp
    regfree(&compiled_regexp);

    //error
    if (status != 0)
        return false;

    //extract infos
    if (value != NULL && matches[2].rm_so != -1)
        *value = atol(command + matches[2].rm_so);
    if (repeat != NULL && matches[3].rm_so != -1)
        *repeat = atol(command + matches[3].rm_so);

    //return status
    return status == 0;
}

bool mem_checker_sim_config_set(mem_checker_t * checker, const char * varname, const char * value)
{
    //check
    assert(checker != NULL);
    assert(varname != NULL);
    assert(value != NULL);

    //set
    #ifdef HAS_MEM_SIM
        return mem_sim_config_set(&checker->simu, varname, value);
    #else
        return false;
    #endif
}

void mem_checker_sim_enable(mem_checker_t * checker)
{
    checker->config.enable_simu = true;
    #ifdef HAS_MEM_SIM
        mem_sim_reset(&checker->simu);
    #endif
}

void mem_checker_sim_show(mem_checker_t * checker)
{
    //error
    if (checker->config.enable_simu == false) {
        CHECKER_PRINT(checker, COLOR_RED "You should enable the simulator with 'Sim_enable' before using this command !" COLOR_RESET "\n");
        abort();
    }

    //call
    #ifdef HAS_MEM_SIM
        mem_sim_show(&checker->simu);
    #endif
}

static void mem_cherkcer_show_print(void * ptr, size_t size, int is_free)
{
    size_t offset = mem_checker_get_print_offset(ptr);
    if (is_free)
        printf("FREE %zu %zu\n", offset, size);
    else
        printf("BUSY %zu %zu\n", offset, size);
}

bool mem_checker_parse_command(mem_checker_t * checker, const char * command)
{
    //vars
    size_t block_size = MEM_UNCHECKED;
    size_t alloc_id = MEM_UNCHECKED;
    size_t repeat = 1;
    size_t i;
    #ifdef HAS_MEM_SIM
        char varname[4096];
        char varvalue[4096];
    #endif

    //copy to trim
    size_t len = strlen(command)+1;
    char * trimed_cmd = malloc(len);
    strncpy(trimed_cmd, command, len);

    //remove line break if has
    char * cursor = trimed_cmd;
    while (*cursor != '\0' && *cursor != '\n')
        cursor++;
    if (*cursor == '\n')
        *cursor = '\0';

    //apply
    if (trimed_cmd[0] == '#') {
        {/*this is comment*/}
    } else if (trimed_cmd[0] == '\0') {
        {/*this is empty line*/}
    } else if (mem_checker_regexp_command(trimed_cmd, "A|Expect_alloc_block|B|Expect_busy_block", &block_size, &repeat)) {
        for (i = 0 ; i < repeat ; i++)
            mem_checker_expect_block(checker, block_size, false);
    } else if (mem_checker_regexp_command(trimed_cmd, "F|Expect_free_block", &block_size, &repeat)) {
        for (i = 0 ; i < repeat ; i++)
            mem_checker_expect_block(checker, block_size, true);
    } else if (mem_checker_regexp_command(trimed_cmd, "S|Expect_skip_block", &block_size, &repeat)) {
        if (block_size == MEM_UNCHECKED)
            return false;
        for (i = 0 ; i < repeat ; i++)
            mem_checker_expect_skiped_block(checker, block_size);
    } else if (mem_checker_regexp_command(trimed_cmd, "I|Ignore_next_blocks", NULL, &repeat)) {
        for (i = 0 ; i < repeat ; i++)
            mem_checker_expect_block(checker, block_size, true);
    } else if (mem_checker_regexp_command(trimed_cmd, "a|Alloc", &block_size, &repeat)) {
        if (block_size == MEM_UNCHECKED)
            return false;
        for (i = 0 ; i < repeat ; i++)
            mem_checker_alloc(checker, block_size);
    } else if (mem_checker_regexp_command(trimed_cmd, "f|Free", &alloc_id, &repeat)) {
        if (alloc_id == MEM_UNCHECKED)
            return false;
        for (i = 0 ; i < repeat ; i++)
            mem_checker_free(checker, alloc_id+i);
    } else if (mem_checker_regexp_command(trimed_cmd, "c|C|Check", NULL, NULL)) {
        mem_checker_check(checker);
    } else if (mem_checker_regexp_command(trimed_cmd, "r|R|Reset", NULL, NULL)) {
        mem_checker_reset(checker, true);
    } else if (strncmp("T ", trimed_cmd, 2) == 0 || strncmp("t ", trimed_cmd, 2) == 0) {
        mem_checker_start_test(checker, trimed_cmd+2);
    } else if (strncmp("Test ", trimed_cmd, 5) == 0) {
        mem_checker_start_test(checker, trimed_cmd+5);
    #ifdef HAS_MEM_SIM
        } else if (mem_checker_regexp_sim_set(trimed_cmd, "Sim_set", varname, varvalue, sizeof(varname))) {
            if (mem_checker_sim_config_set(checker, varname, varvalue) == false)
                return false;
        } else if (strcmp("Sim_enable", trimed_cmd) == 0) {
            mem_checker_sim_enable(checker);
        } else if (strcmp("Check_sim", trimed_cmd) == 0) {
            mem_checker_check_sim(checker);
        } else if (strcmp("Sim_show", trimed_cmd) == 0) {
            mem_checker_sim_show(checker);
    #endif
    } else if (strcmp("Mem_show", trimed_cmd) == 0) {
        mem_checker_mem_show(checker);
    } else {
        free(trimed_cmd);
        return false;
    }

    //ok
    free(trimed_cmd);
    return true;
}

bool mem_checker_check_sim(mem_checker_t * checker)
{
    mem_checker_expect_from_simu(checker);
    return mem_checker_check(checker);
}

void mem_checker_mem_show(mem_checker_t * checker)
{
    mem_show(mem_cherkcer_show_print);
}

void mem_checker_load_script(mem_checker_t * checker, const char * filename)
{
    //check
    assert(checker != NULL);
    assert(filename != NULL);

    //open
    FILE * fp = fopen(filename, "r");
    assert(fp != NULL);

    //loop
    char buffer[4096];
    size_t line = 0;
    while(fgets(buffer, sizeof(buffer), fp) != NULL) {
        line++;
        bool status = mem_checker_parse_command(checker, buffer);
        if (status == false) {
            fprintf(stderr, "Invalid command line=%zu : %s\n", line, buffer);
            exit(1);
        }
    }

    //close
    fclose(fp);
}

static bool mem_checker_compare_block(mem_checker_block_t * expected, mem_checker_block_t * has)
{
    if (expected->offset != MEM_UNCHECKED && expected->offset != has->offset)
        return false;
    if (expected->size != MEM_UNCHECKED && expected->size != has->size)
        return false;
    if (expected->type != has->type)
        return false;
    return true;
}

static void mem_checker_print_block(mem_checker_t * checker, const char * prefix, mem_checker_block_t * block, const char* color)
{
    //check
    assert(block != NULL);

    //handle UNCHECKED
    char size_str[1024] = {'\0'};
    char offset_str[1024] = {'\0'};
    if (block->size != MEM_UNCHECKED && block->type != BLOCK_UNDEFINED)
        sprintf(size_str, "size=%12zu", block->size);
    if (block->offset != MEM_UNCHECKED && block->type != BLOCK_UNDEFINED)
        sprintf(offset_str, "at offset=%12zu,", block->offset);

    if (color == NULL)
        CHECKER_PRINT(checker, "%s%10s block %-23s %-17s", prefix, cst_type_name[block->type], offset_str, size_str);
    else
        CHECKER_PRINT(checker, "%s%s%10s block %-23s %-17s%s", color, prefix, cst_type_name[block->type], offset_str, size_str, COLOR_RESET);
}

static mem_checker_t * gbl_current_checker = NULL;
static void mem_checker_mem_show_print(void * ptr, size_t size, int free)
{
    //check
    assert(gbl_current_checker != NULL);

    //add
    size_t offset = mem_checker_get_offset(ptr);
    mem_checker_add_current_block(gbl_current_checker, offset, size, free);
}

static void mem_checker_mem_show_print_and_check(void * ptr, size_t size, int free)
{
    //check
    assert(gbl_current_checker != NULL);

    //get offset
    size_t offset = mem_checker_get_print_offset(ptr);

    //check that offset is valid
    assert(offset < mem_space_get_size());

    //add
    mem_checker_add_current_block(gbl_current_checker, offset, size, free);

    //check next
    mem_checker_check_next(gbl_current_checker);
}

static void mem_checker_load_mem_show(mem_checker_t * checker, bool check)
{
    //check
    assert(checker != NULL);

    //set current checker to mem_show handler
    gbl_current_checker = checker;

    //call
    if (check)
        mem_show(mem_checker_mem_show_print_and_check);
    else
        mem_show(mem_checker_mem_show_print);

    //reset gbl
    gbl_current_checker = NULL;
}

static bool mem_checker_check_next(mem_checker_t * checker)
{
    //check
    assert(checker != NULL);

    //stop
    if (checker->checking.seen_stop)
        return true;

    //cursor
    size_t i = checker->checking.check_cursor++;

    //set defaults
    mem_checker_block_t expected = {.offset = 0, .size = 0, .type = BLOCK_UNDEFINED};
    mem_checker_block_t current = {.offset = 0, .size = 0, .type = BLOCK_UNDEFINED};

    //extract
    if (i < checker->expected.count)
        expected = checker->expected.blocks[i];
    if (i < checker->current.count)
        current = checker->current.blocks[i];

    //if is stop
    if (expected.type == BLOCK_STOP) {
        checker->checking.seen_stop = true;
        return true;
    }

    //check
    bool status = true;
    if (mem_checker_compare_block(&expected, &current))
    {
        mem_checker_print_block(checker, "==OK== Expected: ", &expected, COLOR_GREEN);
    } else {
        mem_checker_print_block(checker, "==ER== Expected: ", &expected, COLOR_RED);
        mem_checker_print_block(checker, " | Has: ", &current, COLOR_RED);
        checker->checking.error_count++;
        status = false;
    }

    //break
    CHECKER_PRINT(checker, "\n");

    //check range
    if (current.offset + current.size > mem_space_get_size()) {
        CHECKER_PRINT(checker, COLOR_RED "^^^^ OUT OF MEM SPACE RANGE ^^^^" COLOR_RESET "\n");
        checker->checking.error_count++;
        status = false;
    }

    //ret
    return status;
}

static void mem_checker_check_corrupt(mem_checker_t * checker, size_t alloc_id)
{
    //check
    assert(checker != NULL);
    assert(alloc_id < checker->allocs.count);

    //skip
    if (checker->config.check_corrupt == false)
        return;

    //extract
    char canary = checker->allocs.allocs[alloc_id].canary;
    char * cursor = checker->allocs.allocs[alloc_id].ptr;
    size_t size = checker->allocs.allocs[alloc_id].size;

    //skip
    if (cursor == NULL)
        return;

    //check canary
    size_t offset = mem_checker_get_offset(cursor);
    size_t i;
    for (i = 0 ; i < size ; i++) {
        if (cursor[i] != canary) {
            CHECKER_PRINT(checker, COLOR_RED "==ER== Corrupted [%zu] offset=%zu, size=%zu <<< CONTENT HAS BEEN CORRUPTED !" COLOR_RESET "\n", alloc_id, offset, size);
            checker->checking.error_count++;
            break;
        }
    }
}

bool mem_checker_check(mem_checker_t * checker)
{
    //check
    assert(checker != NULL);

    //reset
    checker->checking.check_cursor = 0;
    checker->checking.error_count = 0;
    checker->checking.seen_stop = false;

    //call mem_show
    mem_checker_load_mem_show(checker, true);

    //get cnt max
    size_t cnt_max = checker->current.count;
    if (checker->expected.count > cnt_max)
        cnt_max = checker->expected.count;

    //loop and compare
    size_t i = 0;
    for (i = checker->checking.check_cursor ; i < cnt_max ; i++)
        mem_checker_check_next(checker);

    //check corrupt
    for (i = 0 ; i < checker->allocs.count ; i++)
        if (checker->allocs.allocs[i].ptr != NULL)
            mem_checker_check_corrupt(checker, i);

    //print final status
    if (checker->checking.error_count > 0)
        CHECKER_PRINT(checker, COLOR_RED "[ERROR] Failures=%zu" COLOR_RESET "\n", checker->checking.error_count);
    else
        CHECKER_PRINT(checker, COLOR_GREEN "[SUCCESS]" COLOR_RESET "\n");

    //reset states
    mem_checker_reset(checker, false);

    //ret
    return checker->checking.error_count == 0;
}

void mem_checker_expect_free_block(mem_checker_t * checker, size_t size)
{
    mem_checker_expect_block(checker, size, true);
}

void mem_checker_expect_alloc_block(mem_checker_t * checker, size_t size)
{
    mem_checker_expect_block(checker, size, false);
}

static void mem_checker_expect_block_internal(mem_checker_t * checker, size_t size, mem_checker_block_type_t type)
{
    //check
    assert(checker != NULL);

    //specia case
    if (size == MEM_UNCHECKED && checker->checking.expected_next_offset == 0)
        checker->checking.expected_next_offset = MEM_UNCHECKED;

    //alloc if need
    if (checker->expected.count == checker->expected.allocated) {
        checker->expected.allocated *= 2;
        checker->expected.blocks = realloc(checker->expected.blocks, checker->expected.allocated * sizeof(mem_checker_block_t));
    }

    //add
    size_t cur = checker->expected.count++;
    checker->expected.blocks[cur].type = type;
    checker->expected.blocks[cur].size = size;
    checker->expected.blocks[cur].offset = checker->checking.expected_next_offset;

    //move forward
    if (size == MEM_UNCHECKED || checker->checking.expected_next_offset == MEM_UNCHECKED)
        checker->checking.expected_next_offset = MEM_UNCHECKED;
    else
        checker->checking.expected_next_offset += size;
}

void mem_checker_expect_block(mem_checker_t * checker, size_t size, bool is_free)
{
    if (is_free)
        mem_checker_expect_block_internal(checker, size, BLOCK_FREED);
    else
        mem_checker_expect_block_internal(checker, size, BLOCK_BUSY);
}

void mem_checker_ignore_next_blocks(mem_checker_t * checker)
{
    mem_checker_expect_block_internal(checker, 0, BLOCK_STOP);
}

void mem_checker_expect_skiped_block(mem_checker_t * checker, size_t size)
{
    //check
    assert(checker != NULL);

    //move forward
    checker->checking.expected_next_offset += size;
}

void mem_checker_add_current_block(mem_checker_t * checker, size_t offset, size_t size, bool is_free)
{
    //check
    assert(checker != NULL);

    //alloc if need
    if (checker->current.count == checker->current.allocated) {
        checker->current.allocated *= 2;
        checker->current.blocks = realloc(checker->current.blocks, checker->current.allocated * sizeof(mem_checker_block_t));
    }

    //add
    size_t cur = checker->current.count++;
    checker->current.blocks[cur].type = is_free ? BLOCK_FREED : BLOCK_BUSY;
    checker->current.blocks[cur].size = size;
    checker->current.blocks[cur].offset = offset;
}

static bool mem_checker_alloc_collide(mem_checker_t * checker, size_t id1, size_t id2)
{
    //check
    assert(checker != NULL);
    assert(id1 < checker->allocs.allocated);
    assert(id2 < checker->allocs.allocated);

    //trivial
    if (checker->config.check_collide == false)
        return false;

    //extract
    mem_checker_alloc_t * a1 = &checker->allocs.allocs[id1];
    mem_checker_alloc_t * a2 = &checker->allocs.allocs[id2];

    //check
    if (a1->ptr == NULL || a2->ptr == NULL)
        return false;

    //calc end
    void * end1 = (char*)a1->ptr + a1->size;
    void * end2 = (char*)a2->ptr + a2->size;

    //not collide
     if (end1 <= a2->ptr)
        return false;
    if (end2 <= a1->ptr)
        return false;

    //collide
    return true;
}

size_t mem_checker_alloc(mem_checker_t * checker, size_t alloc_size)
{
    //check
    assert(checker != NULL);

    //id
    size_t id = checker->allocs.count++;

    //grow
    if (id >= checker->allocs.allocated) {
        checker->allocs.allocated *= 2;
        checker->allocs.allocs = realloc(checker->allocs.allocs, checker->allocs.allocated * sizeof(mem_checker_alloc_t));
    }

    //alloc
    void * ptr = mem_alloc(alloc_size);
    size_t offset = mem_checker_get_offset(ptr);
    CHECKER_PRINT(checker, "====== Alloc [id=%zu] offset=%zu, size=%zu \n", id+1, offset, alloc_size);

    //reg
    checker->allocs.allocs[id].ptr = ptr;
    checker->allocs.allocs[id].size = alloc_size;

    //check collision
    for (size_t i = 0 ; i < id ; i++) {
        if (mem_checker_alloc_collide(checker, id, i)) {
            size_t offset1 = mem_checker_get_offset(checker->allocs.allocs[id].ptr);
            size_t offset2 = mem_checker_get_offset(checker->allocs.allocs[i].ptr);
            CHECKER_PRINT(checker, COLOR_PURPLE "WARNING: Allocated block collide another one : %zu[%zu](id=%zu) && %zu[%zu](id=%zu) ! " COLOR_RESET "\n",
                offset1, checker->allocs.allocs[id].size, id,
                offset2, checker->allocs.allocs[i].size, i);
            checker->checking.error_count++;
        }
    }

    //fill with canary
    char canary = rand();
    if (checker->config.check_corrupt && ptr != NULL)
        memset(ptr, canary, alloc_size);

    //set
    checker->allocs.allocs[id].canary = canary;

    //simu
    #ifdef HAS_MEM_SIM
    if (checker->config.enable_simu) {
        size_t sim_offset = (size_t)mem_sim_alloc(&checker->simu, alloc_size);
        if (sim_offset != offset) {
            CHECKER_PRINT(checker, COLOR_PURPLE "Warning: Offset not matching simu expected one : simu=%zu, alloc=%zu" COLOR_RESET "\n", sim_offset, offset);
            //checker->checking.error_count++;
        }
    }
    #endif

    //return the ID
    return id+1;
}

void mem_checker_free(mem_checker_t * checker, size_t alloc_id)
{
    //1->0 convention
    alloc_id--;

    //check
    assert(checker != NULL);
    //assert(alloc_id < checker->allocs.allocs_cnt);

    //check
    if (alloc_id >= checker->allocs.count) {
        CHECKER_PRINT(checker, COLOR_RED "==ER== mem_checker_free() : Invalid allocation id (%zu) " COLOR_RESET "\n", alloc_id);
        checker->checking.error_count++;
        return;
    }

    //check corrupt
    mem_checker_check_corrupt(checker, alloc_id);

    //free
    void * ptr = checker->allocs.allocs[alloc_id].ptr;
    mem_free(ptr);

    //free
    size_t offset = mem_checker_get_offset(ptr);
    size_t size = checker->allocs.allocs[alloc_id].size;
    CHECKER_PRINT(checker, "====== Free  [id=%zu] offset=%zu, size=%zu \n", alloc_id + 1, offset, size);

    //mark freed
    checker->allocs.allocs[alloc_id].ptr = NULL;
    checker->allocs.allocs[alloc_id].size = 0;

    //simu
    #ifdef HAS_MEM_SIM
        if (checker->config.enable_simu)
            mem_sim_free(&checker->simu, (void*)offset);
    #endif
}

static void mem_checker_cut_on_line_break(char * value)
{
    //check
    assert(value != NULL);

    //loop
    while (*value != '\0' && *value != '\n')
        value++;

    //replace
    if (*value == '\n')
        *value =  '\0';
}

static void mem_checker_run_quiet_log(void * arg, const char * message)
{
    //check
    assert(arg != NULL);
    assert(message != NULL);

    //cast
    mem_checker_test_summary_t * summary = (mem_checker_test_summary_t *)arg;

    //skip
    if (summary->display_err == false)
        return;

    //calc size
    size_t sum = strlen(message) + 2;
    if (summary->current_test_log != NULL)
        sum += strlen(summary->current_test_log);

    //realloc
    if (summary->current_test_log == NULL) {
        summary->current_test_log = malloc (sum);
        summary->current_test_log[0] = '\0';
    } else {
        summary->current_test_log = realloc(summary->current_test_log, sum);
    }

    //concat
    strncat(summary->current_test_log, message, sum);
}

static void mem_checker_run_quiet_start(void * arg, const char * name)
{
    //check
    assert(arg != NULL);
    assert(name != NULL);

    //cast
    mem_checker_test_summary_t * summary = (mem_checker_test_summary_t *)arg;

    //skip
    if (summary->quiet == false)
        return;

    //padding
    char pad[4096];
    memset(pad, '.', sizeof(pad));
    pad[QUIET_PADDING - strlen(name)] = '\0';

    //log
    //printf("Running %s%s\n", name, pad);

    //keep track
    size_t len = strlen(name) + 1;
    summary->current_test_name = realloc(summary->current_test_name, len);
    strncpy(summary->current_test_name, name, len);
}

static void mem_checker_run_quiet_end(void * arg, bool status)
{
    //check
    assert(arg != NULL);

    //cast
    mem_checker_test_summary_t * summary = (mem_checker_test_summary_t *)arg;

    //skip
    if (summary->quiet == false)
        return;

    //display log
    if (summary->display_err && status == false)
        printf("%s\n", summary->current_test_log);

    //padding
    char pad[4096];
    memset(pad, '.', sizeof(pad));
    pad[QUIET_PADDING - strlen(summary->current_test_name)] = '\0';

    //print
    if (status)
        printf(COLOR_GREEN "%s%s [SUCCESS]" COLOR_RESET "\n", summary->current_test_name, pad);
    else
        printf(COLOR_RED   "%s%s [FAILURE]" COLOR_RESET "\n", summary->current_test_name, pad);

    //clear logs
    if (summary->current_test_log != NULL)
        summary->current_test_log[0] = '\0';
}

bool mem_checker_run(FILE * fp, bool stop_on_first, bool quiet, bool display_err, const char * fname)
{
    //init alloc
    mem_init();

    //checker
    mem_checker_t checker;
    mem_checker_init(&checker);

    //set summary system if needed
    mem_checker_test_summary_t summary_arg = {NULL, NULL, quiet, display_err};
    if (quiet) {
        mem_checker_set_print(&checker, mem_checker_run_quiet_log, &summary_arg);
        mem_checker_set_test_handler(&checker, mem_checker_run_quiet_start, mem_checker_run_quiet_end, &summary_arg);
    }

    //loop
    char buffer[4096];
    bool final_status = true;
    size_t line = 1;
    while(fgets(buffer, sizeof(buffer), fp) != NULL) {
        mem_checker_cut_on_line_break(buffer);
        bool status = mem_checker_parse_command(&checker, buffer);
        if (!status) {
            fprintf(stderr, COLOR_RED "Invalid command on line %zu : '%s'" COLOR_RESET "\n", line, buffer);
        }
        if (checker.checking.error_count  > 0) {
            final_status = false;
            if(fname != NULL && display_err)
                CHECKER_PRINT((&checker), COLOR_RED "[ERROR] comming from %s:%zu" COLOR_RESET "\n", fname, line);
            if (stop_on_first)
                break;
        }
        line++;
    }

    //cleanup
    mem_checker_fini(&checker);

    //cleanup
    if (summary_arg.current_test_log != NULL)
        free(summary_arg.current_test_log);
    if (summary_arg.current_test_name != NULL)
        free(summary_arg.current_test_name);

    //ok
    return final_status;
}

void mem_checker_set_collide_check(mem_checker_t * checker, bool collide_check)
{
    //check
    assert(checker != NULL);

    //set
    checker->config.check_collide = collide_check;
}

void mem_checker_set_print(mem_checker_t * checker, mem_checker_print_handler_t print, void * arg)
{
    //check
    assert(checker != NULL);

    //set
    checker->config.print = print;
    checker->config.print_arg = arg;
}

void mem_checker_set_test_handler(mem_checker_t * checker, mem_checker_test_start_handler_t on_test_start,
    mem_checker_test_end_handler_t on_test_end, void * arg)
{
    //check
    assert(checker != NULL);

    //set
    checker->config.on_test_start = on_test_start;
    checker->config.on_test_end = on_test_end;
    checker->config.on_test_arg = arg;
}

#ifdef HAS_MEM_SIM
static void mem_checker_sim_handler(mem_sim_block_t * block, void * arg)
{
    //check
    assert(block != NULL);
    assert(arg != NULL);

    //checker
    mem_checker_t * checker = (mem_checker_t *)arg;

    //add
    if (block->block_state == BUSY)
        mem_checker_expect_alloc_block(checker, block->total);
    else
        mem_checker_expect_free_block(checker, block->total);
}
#endif

void mem_checker_expect_from_simu(mem_checker_t * checker)
{
    //check
    assert(checker != NULL);

    //error
    if (checker->config.enable_simu == false) {
        fprintf(stderr, COLOR_RED "You should enable the simulator with 'Sim_enable' before using this command !" COLOR_RESET "\n");
        abort();
    }

    //reset
    mem_checker_reset(checker, false);

    #ifdef HAS_MEM_SIM
        //skip header
        mem_checker_expect_skiped_block(checker, checker->simu.computed.align_header);

        //loop on all blocks
        mem_sim_for_each_block(&checker->simu, mem_checker_sim_handler, checker);
    #endif
}
