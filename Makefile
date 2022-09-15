MAKE=make
TAR=tar
MV=mv

all:
	$(MAKE) -C src

clean:
	$(MAKE) -C src clean

test:
	$(MAKE) -C src test

test_all: all
	$(MAKE) -C src test_all

archive:
	$(MAKE) -C src clean
	$(TAR) -cvjf ../`basename $$PWD`.tar.bz2 --exclude `basename $$PWD`/`basename $$PWD` --exclude .git --exclude *.tar.bz2 --exclude *.tar.gz --exclude googletest-release-* --exclude build* --exclude googletest-usr -C .. `basename $$PWD`
	$(MV) ../`basename $$PWD`.tar.bz2 ./

googletest:
	$(MAKE) -C src googletest

distclean: clean
	$(MAKE) -C src distclean

.PHONY: all clean test test_all archive googletest distclean
