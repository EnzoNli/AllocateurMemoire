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

CXXFLAGS= $(HOST32) -Wall -Werror -std=c++20 -g
CFLAGS= $(HOST32) -Wall -Werror -std=c17 -g -DMEMORY_SIZE=128000
CFLAGS+= -DDEBUG
LDFLAGS= $(HOST32)
INCLUDE=.

PROGRAMS=mem_shell libmalloc.so libmalloc_core_static.a

all: $(PROGRAMS)

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
# pour MAC OS utiliser -install_name à la place de soname
#libmalloc.so: malloc_stub.o $(DEPS)
#	$(CC) -fPIC -shared -Wl,-soname,$@ $^ -o $@
libmalloc.so: malloc_stub.o $(DEPS)
	$(CC) -fPIC -shared -Wl,-install_name,$@ $^ -o $@


# nettoyage
clean:
	rm -f *.o $(PROGRAMS) $(TESTS)

distclean: clean

# deps
mem.c: mem.h mem_os.h mem_space.h
mem_os.c: mem_os.h
mem_space.c: mem_space.h
