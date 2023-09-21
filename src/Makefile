##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2023-2024
##------------------------------------------------------------------------------

.SUFFIXES:

CC=gcc

# uncomment to compile in 32bits mode (require gcc-*-multilib packages
# on Debian/Ubuntu)
#HOST32= -m32

CXXFLAGS= $(HOST32) -Wall -Werror -std=c++20 -g
CFLAGS= $(HOST32) -Wall -Werror -std=c17 -g -DMEMORY_SIZE=128000
CFLAGS+= -DDEBUG
CFLAGS+= -I../headers
CFLAGS+= -DHAS_MEM_SIM
LDFLAGS= $(HOST32)
INCLUDE=.
LIB_DIR=../libs
OS_NAME=$(shell uname -s)
ARCH_NAME=$(shell uname -m)

PROGRAMS=mem_shell libmalloc.so libmalloc_core_static.a 
PROGRAMS+=mem_checker_shell
PROGRAMS+=mem_sim_shell

LIBSIM=libmalloc_sim_$(OS_NAME)_$(ARCH_NAME).a

.PHONY: all place_libs

all: $(PROGRAMS) place_libs

# dépendances des binaires
DEPS=mem.o mem_space.o
DEPS+=mem_checker.o
DEPS+=mem_sim.o

# has sim source
HAS_SIM_SOURCE=$(shell test -f mem_sim.c && echo 1 || echo 0)

%:  $(DEPS) %.o
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -fPIC -c $< $(CFLAGS)

# Extrait le simulateur de la lib statique fournie en binaire
# si on n'a pas les sources
ifeq ($(HAS_SIM_SOURCE), 0)
mem_sim.o: $(LIB_DIR)/$(LIBSIM)
	$(AR) -x $(LIB_DIR)/$(LIBSIM)
endif

# Create static lib to embed in tests
libmalloc_core_static.a: $(DEPS) 
	$(AR) -rcs $@ $^ 

# Create static lib to embed in tests
sim: $(LIBSIM)
$(LIBSIM): mem_sim.o
	$(AR) -rcs $@ $^
	mv $@ $(LIB_DIR)

place_libs: libmalloc_core_static.a libmalloc.so
	mkdir -p $(LIB_DIR);\
	mv libmalloc_core_static.a libmalloc.so $(LIB_DIR);

# test avec des programmes existant
# création d'une librairie partagée
# pour MAC OS utiliser -install_name à la place de soname
libmalloc.so: malloc_stub.o $(DEPS) $(LIB_DIR)/$(LIBSIM)
	$(CC) -fPIC -shared -Wl,-soname,$@ $^ -o $@


# nettoyage
clean:
	rm -f *.o $(PROGRAMS)

distclean: clean
	rm -rf ${LIB_DIR}

# deps
mem.c: ../headers/mem.h ../headers/mem_os.h ../headers/mem_space.h
mem_os.c: ../headers/mem_os.h
mem_space.c: ../headers/mem_space.h
mem_sim.c: ../headers/mem_sim.h ../headers/mem_os.h
