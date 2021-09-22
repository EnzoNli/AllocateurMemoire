#include "mem.h"
#include "mem_os.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define NB_TESTS 10

static void *alloc_max(size_t estimate) {
	void *result;
	static size_t last = 0;

	while ((result = mem_alloc(estimate)) == NULL) {
		estimate--;
	}
	debug("Alloced %zu bytes at %p\n", estimate, result);
	if (last) {
		// Idempotence test
		assert(estimate == last);
	} else
		last = estimate;
	return result;
}

int main(int argc, char *argv[]) {
	fprintf(stderr, "Test réalisant de multiples fois une initialisation suivie d'une alloc max.\n"
			"Définir DEBUG à la compilation pour avoir une sortie un peu plus verbeuse."
 		"\n");
	for (int i=0; i<NB_TESTS; i++) {
		mem_init();
		alloc_max(get_memory_size());
	}

	// TEST OK
	return 0;
}
