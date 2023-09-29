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
	size_t taille_total;
	void *ptr_next_free;
} mem_free_block_t;

typedef struct mem_busy_block_s {
	size_t taille_total;
} mem_busy_block_t;



// Pointeur de fonction sur une des fit fonctions.
typedef mem_free_block_t * (*ptr_fit_function)(mem_free_block_t *,size_t);

// Notre structure d'en-tête
typedef struct allocator_s {
	mem_free_block_t *first_free_block;
	ptr_fit_function actual_fit_function;
	void *debut_mem;
	size_t taille_tot_mem;
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
	mem_free_block_t *first = (mem_free_block_t *) (gbl_alloc + 1);
	first->taille_total = mem_space_get_size() - sizeof(allocator_t);
	first->ptr_next_free = NULL;


	// On initialise tous les attributs de l'allocateur
	gbl_alloc->first_free_block = first;
	gbl_alloc->actual_fit_function = &mem_first_fit;
	gbl_alloc->debut_mem = mem_space_get_addr() + sizeof(allocator_t);
	gbl_alloc->taille_tot_mem = mem_space_get_size();
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
	if(block_address != NULL){
		size_t ancienne_taille = block_address->taille_total;

		// On crée "l'en-tete" du bloc alloué
		mem_busy_block_t *busy_block = (mem_busy_block_t *)block_address;
		busy_block->taille_total = size + sizeof(mem_busy_block_t);

		mem_free_block_t *new_free_block = (mem_free_block_t *) (busy_block + 1);
		new_free_block->taille_total = ancienne_taille - (size + sizeof(mem_busy_block_t));
		new_free_block->ptr_next_free = NULL;
		gbl_alloc->first_free_block = new_free_block;

		// On donne à l'utilisateur l'adresse du bloc alloué (sans l'en-tête du bloc)
		return (void *)(busy_block + sizeof(mem_busy_block_t));
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

/*
void mem_show(void (*print)(void *, size_t, int free)) {
    void *curseur = gbl_alloc + 1;
	mem_free_block_t *block_courant_libre = gbl_alloc->first_free_block;
	mem_busy_block_t *block_courant_alloue;


	while (curseur <= mem_space_get_addr() + gbl_alloc->taille_tot){
		if(curseur == block_courant_libre){
			print(block_courant_libre, block_courant_libre->taille, 1);
			if(block_courant_libre + block_courant_libre->taille == block_courant_libre->ptr_next_free){
				curseur = block_courant_libre->ptr_next_free;
				block_courant_libre = block_courant_libre->ptr_next_free;
			} else {
				curseur = block_courant_libre->ptr_next_free;
				block_courant_alloue = (void *) block_courant_libre + block_courant_libre->taille;
			}
		} else {
			block_courant_alloue = curseur;
			print(block_courant_alloue, block_courant_alloue->taille, 0);
			curseur = block_courant_alloue + block_courant_alloue->taille;
		}
	}
}
*/

int mem_is_free(void *ptr){
	void *curseur_blocks_libres = (void *) gbl_alloc->first_free_block;

	// Parcours de toute la mémoire
	while(curseur_blocks_libres != NULL){
		// Si on depasse le bloc libre actuel => ptr est alloué
		if(ptr < curseur_blocks_libres){
			return 0;
		}

		if(ptr == curseur_blocks_libres){
			return 1;
		}

		// Passage au prochaine bloc libre
		curseur_blocks_libres = ((mem_free_block_t *) curseur_blocks_libres)->ptr_next_free;
	}

	return 0;
}

void mem_show(void (*print)(void *, size_t, int free)) {
    void *curseur = mem_space_get_addr() + sizeof(allocator_t);
	while(curseur < mem_space_get_addr() + gbl_alloc->taille_tot_mem){
		if(mem_is_free(curseur)){
			mem_free_block_t * current_free = ((mem_free_block_t *) curseur);
			print(current_free, current_free->taille_total, 1);
			curseur += current_free->taille_total;
		}else{
			mem_busy_block_t * current_busy = ((mem_busy_block_t *) curseur);
			print(current_busy, current_busy->taille_total, 0);
			curseur += current_busy->taille_total;
		}
	}
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
	if(wanted_size > gbl_alloc->taille_tot_mem){
		return NULL;
	}

    mem_free_block_t *current_block = first_free_block;
    
    while (current_block != NULL) {
        // Vérifie si le bloc courant est suffisamment grand pour la taille demandée
        if (current_block->taille_total >= wanted_size) {
			assert((void *) current_block >= mem_space_get_addr() && (void *) current_block < mem_space_get_addr() + mem_space_get_size());
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
