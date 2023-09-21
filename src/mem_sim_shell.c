//------------------------------------------------------------------------------
// Projet : TP CSE (malloc)
// Cours  : Conception des systèmes d'exploitation et programmation concurrente
// Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
// Année  : 2022-2023
//------------------------------------------------------------------------------

//std c
#include <getopt.h>
//internal
#include "mem_space.h"
#include "mem_sim.h"
#include "mem.h"
#include "mem_os.h"

static void usage(const char *commande) {
    fprintf(stderr, "Usage :\n");
    fprintf(stderr, "%s [ --memsize number ] [ --bbsize number ]"
            "[ --fbsize number ] [ --header number ]"
            "[ --align number ] [ --strategy string ]"
            "[ --split stirng ]"
            "\n\n", commande);
    fprintf(stderr, "Ce programme simule le fonctionnement d'un allocateur mémoire\n"
            "utilisant une stratégie d'allocation contiguë à taille variable.\n"
            "Il accepte comme options \n"
            "--memsize,-M qui donne la taille de la mémoire en octets\n"
            "--bbsize, -B qui donne la taille de la structure pour un bloc occupé\n"
            "--fbsize, -F qui donne la taille de la structure pour un bloc libre\n"
            "--header, -H qui donne la taille de la structure pour l'en-tête\n"
            "--align, -A qui donne le nombre d'octets pour l'alignement mémoire\n"
            "Cet argument est optionnel et par défaut 1 i.e. pas d'alignement.\n"
            "Attention, si vous spécifiez une valeur pour cet argument, \n"
            "ne pas laisser d'espace avec le nom d'option.\n"
            "exemple --align4\n"
            "--strategy, -S qui choisit entre FirstFit (FF), BestFit (BF) et WorstFit (WF)\n"
            "Cet argument est optionnel et par défaut FF.\n"
            "Attention, si vous spécifiez une valeur pour cet argument, \n"
            "ne pas laisser d'espace avec le nom d'option.\n"
            "exemple --strategyBF\n"
            "--split, -s qui choisit la positique de scission, ALLOC_FREE, AF,\n"
            "FREE_ALLOC ou FA en fonction de l'ordre des deux blocs créés.\n"
            "\n");
    exit(1);
}

int main(int argc, char** argv) {
    int opt;
    
    /* -- GET THE OPTIONS -----------------------------------------------------*/
    struct option longopts[] = {
        //{ "memsize",     required_argument, NULL, 'M' },
        { "bbsize", required_argument, NULL, 'B' },
        { "fbsize", required_argument, NULL, 'F' },
        { "header",     required_argument, NULL, 'H' },
        { "align",  optional_argument, NULL, 'A' },
        { "archi",  optional_argument, NULL, 'a' },
        { "strategy",  optional_argument, NULL, 'S' },
        { "split",  optional_argument, NULL, 's' },
        //{ "file",  required_argument, NULL, 'f' }
        
    };

    mem_sim_t sim;
    mem_sim_init(&sim, MEMORY_SIZE);

    //get the command line options
    /* A simple short option is a `-' followed by a short option character. If the option has a required argument, it may be written directly after the option character or as the next parameter (ie. separated by whitespace on the command line). If the option has an optional argument, it must be written directly after the option character if present.
     */
    //while ((opt = getopt_long(argc, (char* const*)argv, "M:B:F:H:AaS::f:", longopts, NULL)) != -1) {
    char buffer[4096];
    while ((opt = getopt_long(argc, (char* const*)argv, "B:M:F:H:Aas:S::", longopts, NULL)) != -1) {
        switch (opt) {
            case 'M':
                mem_sim_config_set(&sim, "MEMORY", optarg);
                mem_sim_config_get(&sim, "MEMORY", buffer, sizeof(buffer));
                debug("SET MEMORY = %s\n", buffer);
                break;
            case 'B':
                mem_sim_config_set(&sim, "BB_SIZE", optarg);
                mem_sim_config_get(&sim, "BB_SIZE", buffer, sizeof(buffer));
                debug("SET BB_SIZE = %s\n", buffer);
                break;
            case 'F':
                mem_sim_config_set(&sim, "FB_SIZE", optarg);
                mem_sim_config_get(&sim, "FB_SIZE", buffer, sizeof(buffer));
                debug("SET FB_SIZE = %s\n", buffer);
                break;
            case 'H':
                mem_sim_config_set(&sim, "HEADER_SIZE", optarg);
                mem_sim_config_get(&sim, "HEADER_SIZE", buffer, sizeof(buffer));
                debug("SET HEADER_SIZE = %s\n", buffer);
                break;
            case 'A':
                mem_sim_config_set(&sim, "ALIGN_SIZE", optarg);
                mem_sim_config_get(&sim, "ALIGN_SIZE", buffer, sizeof(buffer));
                debug("SET ALIGN_SIZE = %s\n", buffer);
                break;
            case 'S':
                //accept FIRST_FIT, FF, BEST_FIT, BF, WORST_FIT, WF
                mem_sim_config_set(&sim, "STRATEGY", optarg);
                mem_sim_config_get(&sim, "STRATEGY", buffer, sizeof(buffer));
                debug("SET STRATEGY = %s\n", buffer);
                break;
            case 's':
                //accept FREE_ALLOC, ALLOC_FREE
                mem_sim_config_set(&sim, "SPLIT_STRATEGY", optarg);
                mem_sim_config_get(&sim, "SPLIT_STRATEGY", buffer, sizeof(buffer));
                debug("SET SPLIT_STRATEGY = %s\n", buffer);
                break;
            default:
                usage(argv[0]);
        }
    }

    /* display config */
    mem_sim_show_config(&sim);
    printf("\n");
    
    /* Initialize empty memory & display simulated state */
    mem_sim_start(&sim);
    mem_sim_show(&sim);

    /* call some malloc/free & display state */
    printf("\n====================== PERFORM OPERATIONS =====================\n");
    void*p = mem_sim_alloc(&sim, 1);
    if (p==NULL) fprintf(stderr, "ECHEC allocation\n");
    printf("===============================================================\n\n");
    mem_sim_show(&sim);

    /* cleanup */
    mem_sim_fini(&sim);

    /* finish */
    return 0;
}
