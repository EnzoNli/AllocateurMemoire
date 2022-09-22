//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

//std C
#include <stdio.h>
#include <string.h>
//internal
#include "mem_checker.h"

void print_help(const char * program_name)
{
	printf("Usage : %s [-c] [FILE1] [FILE2] ...\n\
\n\
Options:\n\
    -c, --continue     Do not stop on first error.\n\
    -q, --quiet        Print output only on error.\n\
    -s, --silent       Print only simple summary.\n\
\n\
Commands:\n\
  Test {NAME}                    Define a new test with the given name. It reset the expected and allocator state.\n\
  T {NAME}\n\
\n\
  Expect_busy_block [SIZE]       Add an expected busy block. Size is optional.\n\
  B [SIZE]\n\
\n\
  Expect_free_block [SIZE]       Add an expected free block. Size is optional.\n\
  F [SIZE]\n\
\n\
  Expect_skip_block [SIZE]       Ignore the given block and increment next expect offset.\n\
  S [SIZE]\n\
\n\
  Ignore_next_blocks             To not check the blocks after this one.\n\
  I\n\
\n\
  Alloc {SIZE}                   Make an allocation of {SIZE} bytes.\n\
  a {SIZE}\n\
\n\
  Free {ALLOC_ID}                Free the block with the given ID (allocation step count, starting from 1)\n\
  f {ALLOC_ID}\n\
\n\
  Check                          Perform a check to search the expected blocks (Expect_*_blocks) in mem_show()\n\
  C\n\
\n\
  Reset                          Reset the testing state.\n\
  R\n\
\n\n\
Remark:\n\
Expect*, Alloc, Free commands also take an optional extra argument to repeat them.\n\
Eg: 'Alloc 8 30x'. \n\
", program_name);
}

int main(int argc, char ** argv)
{
	//vars
	bool status;
	bool stop_on_first = true;
	bool quiet = false;
	bool display_err = true;

	//parse
	int next = 1;
	while (argc > next) {
		//option
		if (strcmp(argv[next],"--continue") == 0 || strcmp(argv[next], "-c") == 0) {
			stop_on_first = false;
			next++;
		} else if (strcmp(argv[next],"--quiet") == 0 || strcmp(argv[next], "-q") == 0) {
			quiet = true;
			next++;
		} else if (strcmp(argv[next],"--silent") == 0 || strcmp(argv[next], "-s") == 0) {
			quiet = true;
			display_err = false;
			next++;
		} else if (strcmp(argv[next],"--help") == 0 || strcmp(argv[next], "-h") == 0) {
			print_help(argv[0]);
			return EXIT_SUCCESS;
		} else {
			break;
		}
	}

	//switch modes
	if (argc <= next) {
		status = mem_checker_run(stdin, stop_on_first, quiet, display_err, NULL);
	} else {
		while (argc > next) {
			const char * fname = argv[next++];
			FILE * fp = fopen(fname, "r");

			//error
			if (fp == NULL) {
				perror(fname);
				return EXIT_FAILURE;
			}

			//run on file
			status = mem_checker_run(fp, stop_on_first, quiet, display_err, fname);
			fclose(fp);

			//stop
			if (stop_on_first && status == false)
				break;
		}
	}

	//end
	if (status)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}
