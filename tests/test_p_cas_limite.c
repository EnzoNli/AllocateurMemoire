#include "mem_space.h"
#include "mem.h"
#include "mem_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main()
{
    void *ptr, *ptr2;
    mem_init();
    // mem_space_get_size() - sizeof(allocator_t) - sizeof(mem_free_block_t))
    // nous n'avons pas defini allocator_t et mem_free_block_t dans les headers, on ne peut donc pas
    // utiliser sizeof(allocator_t), c'est pour cela que je considère que
    // sizeof(allocator_t) = 32 & sizeof(mem_free_block_t) = 16
    assert((ptr = mem_alloc(mem_space_get_size() - 32 - 16)) != NULL);
    printf("Allocation réussie (cas limite).\n");

    printf("Tentative d'allocation avec mémoire saturée.\n");
    assert((ptr2 = mem_alloc(1)) == NULL);
    printf("Allocation échouée (comportement attendu).\n");

    mem_free(ptr);
    printf("Libération de la mémoire.\n");

    return 0;
}
