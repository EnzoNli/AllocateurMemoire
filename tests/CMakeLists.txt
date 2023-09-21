##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2022-2023
##------------------------------------------------------------------------------

######################################################
# include
include_directories(${CMAKE_SOURCE_DIR}/headers)
include_directories(${GTEST_INCLUDE_DIR})

######################################################
# list tests
set(TESTS test_base
          test_base_bzero
          test_cheese
          test_cheese_bzero
          test_frag
          test_fusion
          test_fusion_bzero
)

######################################################
# loop on all tests
foreach(test_name IN LISTS TESTS)
	add_executable(${test_name} ${test_name}.c)
	target_link_libraries(${test_name} malloc_static)
	target_link_libraries(${test_name} ${GTEST_LIBRARIES})
	target_link_libraries(${test_name} pthread)
	add_test(NAME ${test_name} COMMAND c_${test_name})
endforeach()
