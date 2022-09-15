##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2022-2023
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

%: %.c ../libmalloc_core_static.a
	$(CC) $(CFLAGS) --std=c99 -L.. -o $@ $< -lmalloc_core_static

test:
	for file in $(TESTS);do echo; echo ================== $$file =================; echo; ./$$file || exit; done

test_all:
	for file in $(TESTS);do echo "\e[35mRUNING  $$file...\e[0m"; ./$$file > /dev/null 2> /dev/null && echo "\e[32mSUCCESS $$file\e[0m" || echo "\e[31mFAILURE $$file !!!\e[0m"; done

clean:
	$(RM) -f $(PROGRAMS)

distclean: clean

.PHONY: clean test test_all distclean

###############################################################################
#include deps
%.c: ../mem_os.h ../mem_space.h
