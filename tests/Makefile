##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2023-2024
##------------------------------------------------------------------------------

###############################################################################
# commands
CC=gcc

# build flags
CFLAGS= -Wall -Werror -g -DMEMORY_SIZE=128000
#CFLAGS+= -DDEBUG
LDFLAGS=

###############################################################################
# targets
TESTS=test_init test_fusion test_fusion_bzero test_frag test_base test_base_bzero test_cheese test_cheese_bzero 
PROGRAMS=$(TESTS)

###############################################################################
all: $(PROGRAMS)

%: %.c ../src/libmalloc_core_static.a
	$(CC) $(CFLAGS) --std=c17 -L.. -o $@ $< -lmalloc_core_static

test:
	for file in $(TESTS);do echo; echo ================== $$file =================; echo; ./$$file || exit; done

test_all:
	for file in $(TESTS);do echo "\033[0;35mRUNNING  $$file...\033[0m"; ./$$file > /dev/null 2> /dev/null && echo "\033[0;32mSUCCESS $$file\033[0m" || echo "\033[0;31mFAILURE $$file !!!\033[0m"; done

googletest:
	echo "\033[0;35mYou need the advaced branch of the project \033[0m"
	
clean:
	$(RM) -f $(PROGRAMS)

distclean: clean

.PHONY: clean test test_all distclean

###############################################################################
#include deps
%.c: ../src/mem_os.h ../src/mem_space.h
