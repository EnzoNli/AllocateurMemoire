//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "mem.h"
#include "mem_space.h"
#include "mem_os.h"
#include <assert.h>

typedef struct mem_free_block_s {
	size_t taille;
	void *ptr_next_free;
} mem_free_block_t;

typedef struct mem_busy_block_s {
	size_t taille;
} mem_busy_block_t;



// Pointeur de fonction sur une des fit fonctions.
typedef mem_free_block_t * (*ptr_fit_function)(mem_free_block_t *,size_t);

// Notre structure d'en-tête
typedef struct allocator_s {
	mem_free_block_t *first_free_block;
	ptr_fit_function actual_fit_function;
	void *debut_mem;
	size_t taille_tot;
} allocator_t;




allocator_t* gbl_alloc=NULL;

//-------------------------------------------------------------
// mem_init
//-------------------------------------------------------------
/**
 * Initialize the memory allocator.
 * If already init it will re-init.
**/
void mem_init() {
	// On stocke l'allocateur à l'adresse 0
	gbl_alloc = mem_space_get_addr();

	// on crée le premier bloc de mémoire libre
	mem_free_block_t *first = (mem_free_block_t *) (gbl_alloc + sizeof(allocator_t));
	first->taille = mem_space_get_size() - sizeof(allocator_t);
	first->ptr_next_free = NULL;


	// On initialise tous les attributs de l'allocateur
	gbl_alloc->first_free_block = first;
	gbl_alloc->actual_fit_function = &mem_first_fit;
	gbl_alloc->debut_mem = mem_space_get_addr();
	gbl_alloc->taille_tot = mem_space_get_size();
	printf("adresse gbl_alloc = %p, adresse first free block = %p", gbl_alloc, gbl_alloc->first_free_block);
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
/**
 * Allocate a bloc of the given size.
**/
void *mem_alloc(size_t size) {
	// TODO: Mettre à jour le premier bloc libre dans gbl_alloc
	mem_free_block_t *block_address = gbl_alloc->actual_fit_function(gbl_alloc->first_free_block, size + sizeof(mem_busy_block_t));
	if(block_address){
		// On crée "l'en-tete" du bloc alloué
		mem_busy_block_t *busy_block = (mem_busy_block_t *)block_address;
		busy_block->taille = size;

		// On donne à l'utilisateur l'adresse du bloc alloué (sans l'en-tête du bloc)
		return (void *)(busy_block + 1);
	}
    return NULL;
}

//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void * zone)
{
    //TODO: implement
	assert(! "NOT IMPLEMENTED !");
    return 0;
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
/**
 * Free an allocaetd bloc.
**/
void mem_free(void *zone) {
    //TODO: implement
	assert(! "NOT IMPLEMENTED !");
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int free)) {
    void *adresse_test = mem_space_get_addr() + sizeof(allocator_t);
	mem_free_block_t *block_courant_libre = gbl_alloc->first_free_block;
	//mem_busy_block_t *block_courant_alloue;

	printf("adresse_test: %p, block_courant_libre: %p", adresse_test, block_courant_libre);
	/*
	while (adresse_test <= mem_space_get_addr() + gbl_alloc->taille_tot){
		if(adresse_test == block_courant_libre){
			print(block_courant_libre, block_courant_libre->taille, 1);
			if(block_courant_libre + block_courant_libre->taille == block_courant_libre->ptr_next_free){
				adresse_test = block_courant_libre->ptr_next_free;
				block_courant_libre = block_courant_libre->ptr_next_free;
			} else {
				adresse_test = block_courant_libre->ptr_next_free;
				block_courant_alloue = (void *) block_courant_libre + block_courant_libre->taille;
			}
		} else {
			print(block_courant_alloue, block_courant_alloue->taille, 0);
			block_courant_alloue = adresse_test;
			adresse_test = block_courant_alloue + block_courant_alloue->taille;

		}
	}
	*/
}

//-------------------------------------------------------------
// mem_fit
//-------------------------------------------------------------
void mem_set_fit_handler(mem_fit_function_t *mff) {
	gbl_alloc->actual_fit_function = mff;
}

//-------------------------------------------------------------
// Stratégies d'allocation
//-------------------------------------------------------------
mem_free_block_t *mem_first_fit(mem_free_block_t *first_free_block, size_t wanted_size) {
	// Si on essaye d'allouer un espace plus grand que la mémoire totale => impossible
	if(wanted_size > gbl_alloc->taille_tot){
		return NULL;
	}

    mem_free_block_t *current_block = first_free_block;
    
    while (current_block != NULL) {
        // Vérifie si le bloc courant est suffisamment grand pour la taille demandée
        if (current_block->taille >= wanted_size) {
            return current_block;
        }
		// Sinon on passe au bloc libre suivant
        current_block = current_block->ptr_next_free;
    }
    return NULL;
}
//-------------------------------------------------------------
mem_free_block_t *mem_best_fit(mem_free_block_t *first_free_block, size_t wanted_size) {
    //TODO: implement
	assert(! "NOT IMPLEMENTED !");
	return NULL;
}

//-------------------------------------------------------------
mem_free_block_t *mem_worst_fit(mem_free_block_t *first_free_block, size_t wanted_size) {
    //TODO: implement
	assert(! "NOT IMPLEMENTED !");
	return NULL;
}
