MAKE=make
TAR=tar
MV=mv

TESTS_C_DIR_EXISTS=$(shell test -d tests_c && echo 1 || echo 0 )
ifeq ($(TESTS_C_DIR_EXISTS), 1)
	TEST_DIRS=tests tests_c
else
	TEST_DIRS=tests
endif

# compiler les sources
all:
	$(MAKE) -C src

# nettoyer les sources
clean:
	$(MAKE) -C src clean

# compiler les tests
tests: all
	$(MAKE) -C tests all	

test: all
	$(MAKE) -C tests test

test_all: all
	$(MAKE) -C tests test_all

archive:
	$(MAKE) -C src clean
	$(TAR) -cvjf ../`basename $$PWD`.tar.bz2 --exclude `basename $$PWD`/`basename $$PWD` --exclude .git --exclude *.tar.bz2 --exclude *.tar.gz --exclude googletest-release-* --exclude build* --exclude googletest-usr -C .. `basename $$PWD`
	$(MV) ../`basename $$PWD`.tar.bz2 ./

googletest:
	$(MAKE) -C src googletest

distclean: clean
	$(MAKE) -C src distclean

# nettoyage
clean:
ifeq ($(TESTS_C_DIR_EXISTS), 1)
	$(MAKE) -C tests_c clean
endif
	$(MAKE) -C tests clean

.PHONY: clean all googletest test test_all distclean $(TEST_DIRS)
