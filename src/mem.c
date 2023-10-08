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
	// On stocke l'allocateur à l'adresse "0" de notre mémoire
	gbl_alloc = mem_space_get_addr();


	// on crée le premier bloc de mémoire libre
	mem_free_block_t *first = (mem_free_block_t *) (gbl_alloc + 1);
	first->taille_total = mem_space_get_size() - sizeof(allocator_t);
	first->ptr_next_free = NULL;


	// On initialise tous les attributs de l'allocateur
	gbl_alloc->first_free_block = first;
	gbl_alloc->actual_fit_function = &mem_first_fit; //ici qu'on change ??
	gbl_alloc->debut_mem = mem_space_get_addr() + sizeof(allocator_t);
	gbl_alloc->taille_tot_mem = mem_space_get_size();
}

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
//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
/**
 * Allocate a bloc of the given size.
**/
void *mem_alloc(size_t size) {
    // Taille tot nécessaire pour le bloc alloué, y compris l'en-tête
    size_t total_size = size + sizeof(mem_busy_block_t);

    // Recherche du bloc libre assez grand
	//mem_set_fit_handler(mem_fit_function_t *mff);
    mem_free_block_t *free_block = gbl_alloc->actual_fit_function(gbl_alloc->first_free_block, total_size);

    if (free_block != NULL) {
		// Si la taille restante après allocation est inférieure à la taille minimum d'un bloc libre, j'alloue toute la place du bloc
		if(free_block->taille_total - (total_size) < sizeof(mem_free_block_t)){
			total_size=free_block->taille_total;
		}
        // Si le bloc est plus grand que nécessaire, il faut le diviser
        if (free_block->taille_total > total_size) {
            mem_free_block_t *new_free_block = (mem_free_block_t *)((void *)free_block + total_size);

			printf("adresse free_block: %ld\n", (long int)free_block - (long int) mem_space_get_addr());
			printf("adresse new_free_block: %ld\n", (long int)new_free_block - (long int) mem_space_get_addr());

            new_free_block->taille_total = free_block->taille_total - total_size;
            new_free_block->ptr_next_free = free_block->ptr_next_free;

			// si le bloc que je vais allouer est le premier libre alors l'espace en plus devient le nouveau premier libre
			if(free_block==gbl_alloc->first_free_block){
				gbl_alloc->first_free_block = new_free_block;
			}
			//sinon le bloque libre précédent le bloque à alloué prend pour suivant cette espace libre en plus
			else{
				mem_free_block_t *curseur_blocks_libres = gbl_alloc->first_free_block;
				while((mem_free_block_t*)(curseur_blocks_libres->ptr_next_free)<free_block){
					curseur_blocks_libres=curseur_blocks_libres->ptr_next_free;
				}
				curseur_blocks_libres->ptr_next_free=new_free_block;
			}

        }
		// si le bloc que je veux allouer est de la même taille que la zone trouvé
		else{ 
			// et que l'endroit ou j'alloue est le premier bloque libre je met a jour le premier bloque libre de l'allocateur
			if(free_block==gbl_alloc->first_free_block){
				gbl_alloc->first_free_block = free_block->ptr_next_free;
			}
			else{
				mem_free_block_t *curseur_blocks_libres = gbl_alloc->first_free_block;
				while((mem_free_block_t*)(curseur_blocks_libres->ptr_next_free)<free_block){
					curseur_blocks_libres=curseur_blocks_libres->ptr_next_free;
				}
				curseur_blocks_libres->ptr_next_free=free_block->ptr_next_free;

			}
		}
        // Marquer le bloc libre comme alloué
        mem_busy_block_t *busy_block = (mem_busy_block_t *)free_block;
        busy_block->taille_total = total_size;
        // Renvoie l'adresse du bloc alloué (sans l'en-tête)
        return (void *)(busy_block + 1);
    }
    // Aucun bloc libre assez grand n'a été trouvé
    return NULL;
}
//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void * zone){
	// est ce que si espace libre après espace donné doit renvoyer l'addition de deux ?
	size_t espace;
	if (mem_is_free(zone)==1){
		mem_free_block_t* zone_testl=(mem_free_block_t*)zone;
		espace=zone_testl->taille_total;
		while(mem_is_free(zone_testl+zone_testl->taille_total)==1){
			zone_testl=zone_testl+zone_testl->taille_total;
			espace+=zone_testl->taille_total;
		}
	}
	else{
		mem_busy_block_t* zone_testb=(mem_busy_block_t*)zone;
		espace=zone_testb->taille_total;
		while(mem_is_free(zone_testb+zone_testb->taille_total)==1){
			zone_testb=zone_testb+zone_testb->taille_total;
			espace+=zone_testb->taille_total;
		}
	}
	return espace;
    
}
//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
/**
 * Free an allocaetd bloc.
**/
/*void mem_free(void *zone) {
	size_t total_size; 
	mem_free_block_t *first_free_block=gbl_alloc->first_free_block;
	if(mem_is_free(zone)==0){
		mem_busy_block_t *a_liberer=(mem_busy_block_t *)(zone-sizeof(mem_busy_block_t));
		mem_free_block_t *free_block = (mem_free_block_t *)a_liberer;
		mem_free_block_t *prochainlb;
		total_size=a_liberer->taille_total;//pas sur de ça 
		//total_size=*(a_liberer);
		// si il y aucun bloque libre précendent le bloque à liberer
		if(first_free_block>free_block){
			//si le bloque suivant celui à libéré est libre
			if(mem_is_free(a_liberer+total_size)==1){
				total_size+=first_free_block->taille_total;
				//free_block->taille_total=first_free_block->taille_total+total_size;
				//free_block->ptr_next_free=gbl_alloc->first_free_block->ptr_next_free;
				prochainlb=first_free_block->ptr_next_free;
				gbl_alloc->first_free_block=a_liberer;
			}
			else{
				//free_block->ptr_next_free=gbl_alloc->first_free_block;
				prochainlb=first_free_block;
				gbl_alloc->first_free_block=a_liberer;
			}

		}
		else{
			mem_free_block_t *curs_lib=first_free_block;
			//je recupere le dernier bloque libre précédent le bloque à liberer
			while(curs_lib->ptr_next_free<a_liberer){
				curs_lib=curs_lib->ptr_next_free;
			}
			//si le precedent est libre
			if(curs_lib+curs_lib->taille_total==a_liberer){
				total_size+=curs_lib->taille_total;
				a_liberer=a_liberer-curs_lib->taille_total; 
				//et le suivant est libre
				if(a_liberer+total_size==curs_lib->ptr_next_free){
					
					mem_free_block_t *prochain = (mem_free_block_t *)(curs_lib->ptr_next_free);
					total_size+=prochain->taille_total;
					prochainlb=prochain->ptr_next_free;
					//free_block=curs_lib;
					//free_block->taille_total=curs_lib->taille_total+total_size+prochain->taille_total;
				}
				// et le suivant est occupé
				else{
					prochainlb=curs_lib->ptr_next_free;
				}
			}
			//si le précédent est occupé
			else{
				//et le suivant libre
				if(a_liberer+total_size==curs_lib->ptr_next_free){
					mem_free_block_t *prochain = (mem_free_block_t *)(curs_lib->ptr_next_free);
					total_size+=prochain->taille_total;
					prochainlb=prochain->ptr_next_free;
					curs_lib->ptr_next_free=a_liberer;

				}
				// et le suivant est occupé
				else{
					prochainlb=curs_lib->ptr_next_free;
					curs_lib->ptr_next_free=a_liberer;

				}

			}
		}
		// Marquer le bloc alloué comme libre
        
        free_block->taille_total = total_size;
		free_block->ptr_next_free=prochainlb;
	}
	
	
}*/
void fusion_blocs_libres(mem_free_block_t *free_block){
    mem_free_block_t *current_block = gbl_alloc->first_free_block;
    mem_free_block_t *prev_block = NULL;
	mem_free_block_t *prec=gbl_alloc->first_free_block;

    while (current_block != NULL) {
		// Fusion gauche
		printf("fusion current: %ld\n",  (long int) current_block);
		printf("fusion current size: %ld\n", current_block->taille_total);
		printf("fusion free block: %ld\n\n",  (long int) free_block);

        if ((void *)current_block + current_block->taille_total == (void *)free_block) {
            // Fusionne avec le bloc libre suivant
			//free_block = free_block-current_block->taille_total;
			//free_block->taille_total+=current_block->taille_total;
            current_block->taille_total += free_block->taille_total;
			current_block->ptr_next_free=free_block->ptr_next_free;
            free_block = current_block;
			
		// Fusion droite
        }
		if ((void *)free_block + free_block->taille_total == (void *)current_block) {
            // Fusionne avec le bloc libre précédent
            free_block->taille_total += current_block->taille_total;

            // Ajoute le bloc fusionné à la liste
            if (prev_block != NULL) {
				while(prec->ptr_next_free<(void *)free_block){
					prec=prec->ptr_next_free;
				}
				if(prec!=free_block){
					prec->ptr_next_free=free_block;
				}
				//prec->ptr_next_free=free_block;
				
                //prev_block->ptr_next_free = current_block->ptr_next_free;
				free_block->ptr_next_free=current_block->ptr_next_free;
            } else {
                gbl_alloc->first_free_block = current_block;
				free_block->ptr_next_free=current_block->ptr_next_free;
            }
        }

        prev_block = current_block;
        current_block = current_block->ptr_next_free;
    }
}


void mem_free(void *zone) {

	if (zone == NULL || mem_is_free(zone - sizeof(mem_busy_block_t))) {
        // Ne rien faire, la zone est déjà un bloc libre
        return;
    }

    // Récupère l'adresse du header du bloc occupé
    mem_busy_block_t *busy_block = (mem_busy_block_t *)(zone - sizeof(mem_busy_block_t));
	mem_free_block_t *prev_block = gbl_alloc->first_free_block;

    // "Ecrase" le bloc alloué par un bloc libre
    mem_free_block_t *free_block = (mem_free_block_t *)busy_block;
    free_block->taille_total = busy_block->taille_total;

	if(prev_block>free_block){
		free_block->ptr_next_free=prev_block;
		gbl_alloc->first_free_block=free_block;
		fusion_blocs_libres(free_block);
	}
	else if (prev_block != NULL){

		while(prev_block->ptr_next_free != NULL && prev_block->ptr_next_free <= (void *) free_block){
			prev_block = prev_block->ptr_next_free;
		}

    	free_block->ptr_next_free = prev_block->ptr_next_free;
		prev_block->ptr_next_free = free_block;

		/*if(gbl_alloc->first_free_block > free_block){
    		gbl_alloc->first_free_block = free_block;
		}*/

    	fusion_blocs_libres(free_block);
	}else{
		free_block->ptr_next_free=NULL;
		gbl_alloc->first_free_block=free_block;
	}

}


//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
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
	mem_free_block_t *best_block;
    if(wanted_size > gbl_alloc->taille_tot_mem){
		return NULL;
	}

    mem_free_block_t *current_block = first_free_block;
    // on recupère le premier bloque assez grand
    while (current_block != NULL && current_block->taille_total < wanted_size) {
        current_block = current_block->ptr_next_free;
    }
	if(current_block != NULL){
	best_block = current_block;
	current_block = current_block->ptr_next_free;
	}
	else{
		printf("Place insuffisante!!!");
    	return NULL;
	}
	while (current_block != NULL) {
        // Vérifie si le bloc courant est suffisamment grand pour la taille demandée 
		//et est plus petit que le bloc assez grand précédent
        if (current_block->taille_total >= wanted_size && current_block->taille_total < best_block->taille_total) {
            best_block=current_block;
        }
        current_block = current_block->ptr_next_free;
    }
	return best_block;
}

//-------------------------------------------------------------
mem_free_block_t *mem_worst_fit(mem_free_block_t *first_free_block, size_t wanted_size) {
	if(wanted_size > gbl_alloc->taille_tot_mem){
		return NULL;
	}

    mem_free_block_t *current_block = first_free_block;
	mem_free_block_t *best_block;

	// on recupère le premier bloque assez grand
    while (current_block != NULL && current_block->taille_total< wanted_size) {
        current_block = current_block->ptr_next_free;
    }
	// si il existe il est pour l'instant le bloque choisi pour allouer l'espace demandé
	if(current_block != NULL){
	best_block = current_block;
	current_block = current_block->ptr_next_free;
	}
	// sinon c'est que aucun espace libre ne permet d'allouer la place demandé
	else{
		printf("Place insuffisante!!!");
    	return NULL;
	}
    // on parcours l'ensemble des bloques libres à la recherche du plus grand pouvant stoqué l'espace demandé
    while (current_block != NULL) {
        //  si le bloc courant est plus grand que le plus grand actuel alors il devient le bloque le plus grand
        if (current_block->taille_total>best_block->taille_total ) {
			best_block= current_block;
        }
		//puis on regarde le bloque libre suivant
        current_block = current_block->ptr_next_free;
    }
	return best_block;
}