//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
//------------------------------------------------------------------------------

#include "mem.h"
#include "mem_space.h"
#include "mem_os.h"
#include <assert.h>

typedef struct mem_free_block_s
{
	size_t taille_total;
	void *ptr_next_free;
} mem_free_block_t;

typedef struct mem_busy_block_s
{
	size_t taille_total;
} mem_busy_block_t;

// Pointeur de fonction sur une des fit fonctions.
typedef mem_free_block_t *(*ptr_fit_function)(mem_free_block_t *, size_t);

// Notre structure d'en-tête
typedef struct allocator_s
{
	mem_free_block_t *first_free_block;
	ptr_fit_function actual_fit_function;
	void *debut_mem;
	size_t taille_tot_mem;
} allocator_t;

allocator_t *gbl_alloc = NULL;

//-------------------------------------------------------------
// mem_init
//-------------------------------------------------------------
/**
 * Initialize the memory allocator.
 * If already init it will re-init.
 **/
void mem_init()
{
	// On stocke l'allocateur à l'adresse "0" de notre mémoire
	gbl_alloc = mem_space_get_addr();

	// on crée le premier bloc de mémoire libre
	mem_free_block_t *first = (mem_free_block_t *)(gbl_alloc + 1);
	first->taille_total = mem_space_get_size() - sizeof(allocator_t);
	first->ptr_next_free = NULL;

	// On initialise tous les attributs de l'allocateur
	gbl_alloc->first_free_block = first;
	gbl_alloc->actual_fit_function = &mem_first_fit;
	gbl_alloc->debut_mem = mem_space_get_addr();
	gbl_alloc->taille_tot_mem = mem_space_get_size();
}

//-------------------------------------------------------------
// mem_is_free
//-------------------------------------------------------------
/**
 * Permet de savoir si oui ou non le pointeur passé en paramètre se trouve dans une zone libre
 **/
int mem_is_free(void *ptr)
{
	void *curseur_blocks_libres = (void *)gbl_alloc->first_free_block;

	// Parcours de toute la mémoire
	while (curseur_blocks_libres != NULL)
	{
		// Si on depasse le bloc libre actuel => ptr est alloué
		if (ptr < curseur_blocks_libres)
		{
			return 0;
		}

		// Si le pointeur pointe sur un bloc libre => le pointeur est dans une zone libre
		if (ptr == curseur_blocks_libres)
		{
			return 1;
		}

		// Passage au prochaine bloc libre
		curseur_blocks_libres = ((mem_free_block_t *)curseur_blocks_libres)->ptr_next_free;
	}

	return 0;
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
/**
 * Allocate a bloc of the given size.
 **/
void *mem_alloc(size_t size)
{
	// Taille tot nécessaire pour le bloc alloué, y compris l'en-tête & alignement
	// Alignement de la taille sur 4 octets en utilisant un masque (3 en binaire
	// est 11, ~3 sera donc 11111100 en binaire) On force donc les deux derniers
	// bits de total_size_a_allouer à zéro, cela permet de s'assurer ainsi qu'il
	// est aligné sur 4 octets.
	size_t total_size_a_allouer = (size + sizeof(mem_busy_block_t) + 3) & ~3;
	// size_t total_size_a_allouer = size + sizeof(mem_busy_block_t);
	// size_t mod=total_size_a_allouer%4;
	// if(mod==0){
	//	total_size_a_allouer+=mod;
	// }

	// Si la taille totale à allouer est plus petite que l'entête d'un bloc libre, alors la taille a allouer devient la taille de l'entête d'un bloc libre
	if (total_size_a_allouer < sizeof(mem_free_block_t))
	{
		total_size_a_allouer = sizeof(mem_free_block_t);
	}

	// Si l'utilisateur essaye d'allouer une taille plus grande que la taille de la mémoire => echec de l'allocation
	if (total_size_a_allouer > gbl_alloc->taille_tot_mem - sizeof(allocator_t) - sizeof(mem_busy_block_t))
	{
		return NULL;
	}
	// Recherche du bloc libre assez grand
	mem_free_block_t *free_block = gbl_alloc->actual_fit_function(gbl_alloc->first_free_block, total_size_a_allouer);

	if (free_block != NULL)
	{
		// Si la taille restante après allocation est inférieure à la taille minimum d'un bloc libre, j'alloue toute la place du bloc
		if (free_block->taille_total - (total_size_a_allouer) < sizeof(mem_free_block_t))
		{
			total_size_a_allouer = free_block->taille_total;
		}

		// Si le bloc est plus grand que nécessaire, il faut le diviser
		if (free_block->taille_total > total_size_a_allouer)
		{
			mem_free_block_t *update_ptr_next = free_block->ptr_next_free;
			size_t update_size = free_block->taille_total;
			mem_free_block_t *new_free_block = (mem_free_block_t *)((void *)free_block + total_size_a_allouer);

			new_free_block->taille_total = update_size - total_size_a_allouer;
			new_free_block->ptr_next_free = update_ptr_next;

			// Si l'ancien bloc libre était le premier bloc libre de la mémoire alors l'espace en plus devient le nouveau premier bloc libre
			if (free_block == gbl_alloc->first_free_block)
			{
				gbl_alloc->first_free_block = new_free_block;
			}
			// sinon le bloc libre précédent au bloc à alloué prend pour suivant cette espace libre en plus
			else
			{
				mem_free_block_t *curseur_blocks_libres = gbl_alloc->first_free_block;
				while ((mem_free_block_t *)(curseur_blocks_libres->ptr_next_free) < free_block)
				{
					curseur_blocks_libres = curseur_blocks_libres->ptr_next_free;
				}
				curseur_blocks_libres->ptr_next_free = new_free_block;
			}
		}
		// si le bloc que je veux allouer est de la même taille que la zone trouvé
		else
		{
			// Si l'ancien bloc libre était le premier bloc libre de la mémoire alors l'espace en plus devient le nouveau premier bloc libre
			if (free_block == gbl_alloc->first_free_block)
			{
				gbl_alloc->first_free_block = free_block->ptr_next_free;
			}
			else
			{
				// sinon le bloc libre précédent au bloc à alloué prend pour suivant cette espace libre en plus
				mem_free_block_t *curseur_blocks_libres = gbl_alloc->first_free_block;
				while ((mem_free_block_t *)(curseur_blocks_libres->ptr_next_free) < free_block)
				{
					curseur_blocks_libres = curseur_blocks_libres->ptr_next_free;
				}
				curseur_blocks_libres->ptr_next_free = free_block->ptr_next_free;
			}
		}
		// Marquer le bloc libre comme alloué
		mem_busy_block_t *busy_block = (mem_busy_block_t *)free_block;
		busy_block->taille_total = total_size_a_allouer;
		// Renvoie l'adresse du bloc alloué (sans l'en-tête)
		return (void *)(busy_block + 1);
	}
	// Aucun bloc libre assez grand n'a été trouvé
	return NULL;
}

//-------------------------------------------------------------
// mem_get_size
//-------------------------------------------------------------
size_t mem_get_size(void *zone)
{
	size_t espace;
	if (mem_is_free(zone) == 1)
	{
		return 0;
	}
	else
	{
		mem_busy_block_t *zone_testb = (mem_busy_block_t *)zone;
		espace = zone_testb->taille_total-sizeof(mem_busy_block_t);
		return espace;
	}
	
}

//-------------------------------------------------------------
// fusion_blocs_libres
//-------------------------------------------------------------
/**
 * Premet de fusionner le bloc libre passé en paramètre avec ses voisins si ils sont libres
 **/
void fusion_blocs_libres(mem_free_block_t *free_block)
{
	mem_free_block_t *current_block = gbl_alloc->first_free_block;
	mem_free_block_t *prev_block = NULL;
	mem_free_block_t *prec = gbl_alloc->first_free_block;

	while (current_block != NULL)
	{
		// Fusion gauche
		if ((void *)current_block + current_block->taille_total == (void *)free_block)
		{
			// Fusionne avec le bloc libre suivant
			current_block->taille_total += free_block->taille_total;
			current_block->ptr_next_free = free_block->ptr_next_free;
			free_block = current_block;
		}

		// Fusion droite
		if ((void *)free_block + free_block->taille_total == (void *)current_block)
		{
			// Fusionne avec le bloc libre précédent
			free_block->taille_total += current_block->taille_total;

			// Ajoute le bloc fusionné à la liste
			if (prev_block != NULL)
			{
				while (prec->ptr_next_free < (void *)free_block)
				{
					prec = prec->ptr_next_free;
				}
				if (prec != free_block)
				{
					prec->ptr_next_free = free_block;
				}
				free_block->ptr_next_free = current_block->ptr_next_free;
			}
			else
			{
				gbl_alloc->first_free_block = current_block;
				free_block->ptr_next_free = current_block->ptr_next_free;
			}
		}

		prev_block = current_block;
		current_block = current_block->ptr_next_free;
	}
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int free))
{
	void *curseur = gbl_alloc->debut_mem + sizeof(allocator_t);
	while (curseur < gbl_alloc->debut_mem + gbl_alloc->taille_tot_mem)
	{
		if (mem_is_free(curseur))
		{
			mem_free_block_t *current_free = ((mem_free_block_t *)curseur);
			print(current_free, current_free->taille_total, 1);
			curseur += current_free->taille_total;
		}
		else
		{
			mem_busy_block_t *current_busy = ((mem_busy_block_t *)curseur);
			print(current_busy, current_busy->taille_total, 0);
			curseur += current_busy->taille_total;
		}
	}
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
/**
 * Free an allocaetd bloc.
 **/

void mem_free(void *zone)
{

	if (zone == NULL || mem_is_free(zone - sizeof(mem_busy_block_t)))
	{
		// Ne rien faire, la zone est déjà un bloc libre
		return;
	}

	// Récupère l'adresse du header du bloc occupé
	mem_busy_block_t *busy_block = (mem_busy_block_t *)(zone - sizeof(mem_busy_block_t));
	mem_free_block_t *prev_block = gbl_alloc->first_free_block;

	// "Ecrase" le bloc alloué par un bloc libre
	mem_free_block_t *free_block = (mem_free_block_t *)busy_block;
	free_block->taille_total = busy_block->taille_total;

	if (prev_block > free_block)
	{
		free_block->ptr_next_free = prev_block;
		gbl_alloc->first_free_block = free_block;
		fusion_blocs_libres(free_block);
	}
	else if (prev_block != NULL)
	{

		while (prev_block->ptr_next_free != NULL && prev_block->ptr_next_free < (void *)free_block)
		{
			prev_block = prev_block->ptr_next_free;
		}

		free_block->ptr_next_free = prev_block->ptr_next_free;
		prev_block->ptr_next_free = free_block;

		fusion_blocs_libres(free_block);
	}
	else
	{
		free_block->ptr_next_free = NULL;
		gbl_alloc->first_free_block = free_block;
	}
}

void *mem_realloc(void *ptr, size_t taille) // j'suis pas sur d'etre ok
{
	// Si size = zéro, cela équivaut à un mem_free(ptr)
	if (taille == 0)
	{
		mem_free(ptr);
		return NULL;
	}

	// Si ptr = NULL, cela équivaut à un mem_alloc(size)
	if (ptr == NULL)
	{
		return mem_alloc(taille);
	}

	// Récupère la taille actuelle de la zone allouée
	size_t ancienne_taille = mem_get_size(ptr);
	// Alloue une nouvelle zone de mémoire de taille size
	void *nouveau_ptr = mem_alloc(taille);

	// Mettre à jour les pointeurs
	mem_free_block_t *prec = gbl_alloc->first_free_block;
	while (prec->ptr_next_free < (void *)ptr)
	{
		prec = prec->ptr_next_free;
	}
	prec->ptr_next_free = nouveau_ptr;

	if (nouveau_ptr != NULL)
	{
		size_t taille_donnees_a_copier;

		if (ancienne_taille < taille)
		{
			taille_donnees_a_copier = ancienne_taille;
		}
		else
		{
			taille_donnees_a_copier = taille;
		}

		// Copie les données de l'ancienne zone vers la nouvelle zone
		char *ancienne_data = (char *)ptr;
		char *nouvelle_data = (char *)taille;

		for (size_t i = 0; i < taille_donnees_a_copier; ++i)
		{
			nouvelle_data[i] = ancienne_data[i];
		}

		// Libère l'ancienne zone
		mem_free(ptr);
	}

	return nouveau_ptr;
}

//-------------------------------------------------------------
// mem_fit
//-------------------------------------------------------------
void mem_set_fit_handler(mem_fit_function_t *mff)
{
	gbl_alloc->actual_fit_function = mff;
}

//-------------------------------------------------------------
// Stratégies d'allocation
//-------------------------------------------------------------
mem_free_block_t *mem_first_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
	// Si on essaye d'allouer un espace plus grand que la mémoire totale => impossible
	if (wanted_size > gbl_alloc->taille_tot_mem)
	{
		return NULL;
	}

	mem_free_block_t *current_block = first_free_block;

	while (current_block != NULL)
	{
		//  Vérifie si le bloc courant est suffisamment grand pour la taille demandée
		if (current_block->taille_total >= wanted_size)
		{
			assert((void *)current_block >= gbl_alloc->debut_mem && (void *)current_block < gbl_alloc->debut_mem + gbl_alloc->taille_tot_mem);
			return current_block;
		}
		// Sinon on passe au bloc libre suivant
		current_block = current_block->ptr_next_free;
	}
	return NULL;
}
//-------------------------------------------------------------
mem_free_block_t *mem_best_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
	mem_free_block_t *best_block;
	if (wanted_size > gbl_alloc->taille_tot_mem)
	{
		return NULL;
	}

	mem_free_block_t *current_block = first_free_block;

	// on recupère le premier bloc assez grand
	while (current_block != NULL && current_block->taille_total < wanted_size)
	{
		current_block = current_block->ptr_next_free;
	}
	if (current_block != NULL)
	{
		best_block = current_block;
		current_block = current_block->ptr_next_free;
	}
	else
	{
		// Place insuffisante
		return NULL;
	}
	while (current_block != NULL)
	{
		// Vérifie si le bloc courant est suffisamment grand pour la taille demandée
		// et est plus petit que le bloc assez grand précédent
		if (current_block->taille_total >= wanted_size && current_block->taille_total < best_block->taille_total)
		{
			best_block = current_block;
		}
		current_block = current_block->ptr_next_free;
	}
	return best_block;
}

//-------------------------------------------------------------
mem_free_block_t *mem_worst_fit(mem_free_block_t *first_free_block, size_t wanted_size)
{
	if (wanted_size > gbl_alloc->taille_tot_mem)
	{
		return NULL;
	}

	mem_free_block_t *current_block = first_free_block;
	mem_free_block_t *best_block;

	// on recupère le premier bloc assez grand
	while (current_block != NULL && current_block->taille_total < wanted_size)
	{
		current_block = current_block->ptr_next_free;
	}
	// si il existe, il est pour l'instant le bloc choisi pour allouer l'espace demandé
	if (current_block != NULL)
	{
		best_block = current_block;
		current_block = current_block->ptr_next_free;
	}
	// sinon c'est qu'aucun espace libre ne permet d'allouer la place demandée
	else
	{
		// Place insuffisante
		return NULL;
	}
	// on parcours l'ensemble des blocs libres à la recherche du plus grand pouvant stocker l'espace demandé
	while (current_block != NULL)
	{
		//  si le bloc courant est plus grand que le plus grand actuel alors il devient le bloc le plus grand
		if (current_block->taille_total > best_block->taille_total)
		{
			best_block = current_block;
		}
		// puis on regarde le bloc libre suivant
		current_block = current_block->ptr_next_free;
	}
	return best_block;
}