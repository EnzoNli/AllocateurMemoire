##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2022-2023
##------------------------------------------------------------------------------

######################################################
# Ajoute quelques options pour tous les builds
include_directories(${CMAKE_SOURCE_DIR}/headers)
add_compile_options(-Wall)
add_compile_options(-Werror)
add_definitions(-DHAS_MEM_SIM)
add_definitions(-DMEMORY_SIZE=${MEMORY_SIZE})

######################################################
# Liste les sources à compiler pour former la bibliothèque
set(MEM_SRC mem.c mem_space.c mem_checker.c)

######################################################
# Compilation des deux librairies, dynamique (pour LD_PRELOAD) et statique pour
# les tests
add_library(malloc SHARED ${MEM_SRC} malloc_stub.c)
add_library(malloc_static STATIC ${MEM_SRC})
target_link_libraries(malloc_static ${CMAKE_SOURCE_DIR}/libs/libmalloc_sim_Linux_x86_64.a)

######################################################
add_executable(mem_shell mem_shell.c)
target_link_libraries(mem_shell malloc_static)

######################################################
add_executable(mem_checker_shell mem_checker_shell.c)
target_link_libraries(mem_checker_shell malloc_static)

