##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2022-2023
##------------------------------------------------------------------------------

.SUFFIXES:

CC=gcc

# uncomment to compile in 32bits mode (require gcc-*-multilib packages
# on Debian/Ubuntu)
#HOST32= -m32

CXXFLAGS= $(HOST32) -Wall -Werror -std=c++11 -g
CFLAGS= $(HOST32) -Wall -Werror -std=c99 -g -DMEMORY_SIZE=128000
CFLAGS+= -DDEBUG
LDFLAGS= $(HOST32)
INCLUDE=.

PROGRAMS=mem_shell libmalloc.so libmalloc_core_static.a

TESTS_C_DIR_EXISTS=$(shell test -d tests_c && echo 1 || echo 0 )
ifeq ($(TESTS_C_DIR_EXISTS), 1)
	TEST_DIRS=tests tests_c
else
	TEST_DIRS=tests
endif

.PHONY: clean all googletest test test_all distclean $(TEST_DIRS)

all: $(PROGRAMS) $(TEST_DIRS)

tests:
	$(MAKE) -C tests

tests_c:
	$(MAKE) -C tests_c

googletest:
	$(MAKE) -C tests googletest

test: all
	$(MAKE) -C tests test

test_all: all
	$(MAKE) -C tests test_all

# dépendances des binaires
DEPS=mem.o mem_space.o

%:  $(DEPS) %.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -fPIC -c $< $(CFLAGS)

# Create static lib to embed in tests
libmalloc_core_static.a: $(DEPS)
	$(AR) -rcs $@ $^

# test avec des programmes existant
# création d'une librairie partagée
libmalloc.so: malloc_stub.o $(DEPS)
	$(CC) -fPIC -shared -Wl,-soname,$@ $^ -o $@

# nettoyage
clean:
ifeq ($(TESTS_C_DIR_EXISTS), 1)
	$(MAKE) -C tests_c clean
endif
	$(MAKE) -C tests clean
	rm -f *.o $(PROGRAMS) $(TESTS)

distclean: clean
	$(MAKE) -C tests distclean

# deps
mem.c: mem.h mem_os.h mem_space.h
mem_os.c: mem_os.h
mem_space.c: mem_space.h
