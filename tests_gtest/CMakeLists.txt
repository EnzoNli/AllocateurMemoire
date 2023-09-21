##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2022-2023
##------------------------------------------------------------------------------

######################################################
# Ajoute le dossier include de GTest (optiopn gcc -I......)
include_directories(${GTEST_INCLUDE_DIR})

######################################################
add_definitions(-DSEQ_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../test_shell_sequences/")

######################################################
# liste des tests
set(TESTS test_base
          test_basic
          test_cheese
          test_frag
          test_fusion
          test_init
          test_unit
)

######################################################
# Fichier optionel
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test_validate_all.cpp)
	list(APPEND TEST test_validate_all)
endif()

######################################################
# Boucle sur les tests et définit leur build et enregistre le test
foreach(test_name IN LISTS TESTS)
	# ajoute un executable à compiler
	add_executable(g${test_name} ${test_name}.cpp)
	# ajoute les bibliothèques à l'exécutable
	target_link_libraries(g${test_name} malloc_static)
	target_link_libraries(g${test_name} ${GTEST_LIBRARIES})
	target_link_libraries(g${test_name} pthread)
	# enregistre le test pour "make test" et "ctest"
	add_test(NAME g${test_name} COMMAND ${test_name})
endforeach()
