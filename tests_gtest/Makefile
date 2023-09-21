##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2023-2024
##------------------------------------------------------------------------------

###############################################################################
# commands
CXX=g++
MAKE=make
MKDIR=mkdir
CMAKE=cmake

# build flags
CXXFLAGS= -Wall -Werror -std=c++11 -g
CFLAGS= -Wall -Werror -g -DMEMORY_SIZE=128000 "-DSEQ_DIR=\"$(CURDIR)/../tests_shell_sequences/\""
#CFLAGS+= -DDEBUG
LDFLAGS=

###############################################################################
# set derivated vars
GTEST_PREFIX=./googletest-usr
GTEST_CXXFLAGS=-I$(GTEST_PREFIX)/include
GTEST_LDFLAGS=-L$(GTEST_PREFIX)/lib -lgtest_main -lgtest -lpthread

###############################################################################
# targets
TESTS=test_unit
TESTS+=test_init
TESTS+=test_basic
TESTS+=test_base
TESTS+=test_fusion
TESTS+=test_frag
TESTS+=test_cheese

# list of programs to build
PROGRAMS=$(TESTS)

###############################################################################
all: $(PROGRAMS)

%: %.cpp ../libs/libmalloc_core_static.a googletest
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(GTEST_CXXFLAGS) --std=c++11 -L../libs -o $@ $< $(GTEST_LDFLAGS) -lmalloc_core_static

test:
	for file in $(TESTS);do echo; echo ================== $$file =================; echo; ./$$file || exit; done

test_all:
	for file in $(TESTS);do echo "\033[0;35mRUNNING  $$file...\033[0m"; ./$$file > /dev/null 2> /dev/null && echo "\033[0;32mSUCCESS $$file\033[0m" || echo "\033[0;31mFAILURE $$file !!!\033[0m"; done

clean:
	$(RM) -f $(PROGRAMS)

distclean: clean
	$(RM) -f google-test-1.12.1.tar.gz
	$(RM) -rf googletest-release-1.12.1
	$(RM) -rf googletest-usr

###############################################################################
googletest: googletest-usr/usr/include/gtest/gtest.h

google-test-1.12.1.tar.gz:
	wget https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz -O google-test-1.12.1.tar.gz

googletest-release-1.12.1: google-test-1.12.1.tar.gz
	tar -xvf google-test-1.12.1.tar.gz

googletest-usr/usr/include/gtest/gtest.h: googletest-release-1.12.1
	$(MKDIR) -p googletest-release-1.12.1/build
	DIR=$$PWD; cd googletest-release-1.12.1/build && $(CMAKE) .. -DCMAKE_INSTALL_PREFIX=$$DIR/googletest-usr
	cd googletest-release-1.12.1/build && $(MAKE)
	cd googletest-release-1.12.1/build && $(MAKE) install

.PHONY: clean test test_all distclean

###############################################################################
#include deps
%.cpp: ../headers/mem_os.h ../headers/mem_space.h ../headers/mem.h
