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
set(TESTS_C test_basic)
set(TESTS_SHELL test_basic)

######################################################
# loop on all tests
foreach(test_name IN LISTS TESTS_C)
	add_executable(${test_name} ${test_name}.c)
	target_link_libraries(${test_name} malloc_static)
	target_link_libraries(${test_name} ${GTEST_LIBRARIES})
	target_link_libraries(${test_name} pthread)
	add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()

######################################################
# loop on all tests
foreach(test_name IN LISTS TESTS_SHELL)
	add_test(NAME ${test_name}_shell COMMAND $<TARGET_FILE:mem_checker_shell> ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.check)
endforeach()
