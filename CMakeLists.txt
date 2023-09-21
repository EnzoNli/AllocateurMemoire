##------------------------------------------------------------------------------
## Projet : TP CSE (malloc)
## Cours  : Conception des systèmes d'exploitation et programmation concurrente
## Cursus : Université Grenoble Alpes - UFRIM²AG - Master 1 - Informatique
## Année  : 2022-2023
##------------------------------------------------------------------------------

######################################################
# Définit la version minimum de cmake requise
cmake_minimum_required(VERSION 3.0)
# Definit le nom et la version du projet ainsi que les languages à activer
project(tp-allocator VERSION 0.0.0 LANGUAGES CXX C)
# Active the support des tests (make test / ctest)
enable_testing()

######################################################
# Configuration globale, si on veux changer les valeures par default on peut utiliser :
# cmake -DENABLE_TESTS=OFF -DMEMORY_SIZE 256000 ..
set(ENABLE_TESTS ON CACHE BOOL "Enable unit tests requiring build of googletest and googlmock")
set(MEMORY_SIZE 128000 CACHE STRING "Define the maximum amount of memory handled by the allocator")

######################################################
# Si les tests sont activés on télécharge les sources de googletest et on l'ajoute au build
# via add_subdirectory car googletest se build aussi via cmake.
if (ENABLE_TESTS)
	# Télécharge les sources si non encore fait
	if(NOT IS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/deps/googletest-release-1.12.1)
		message(STATUS "Downloading Google Test...")
		execute_process(COMMAND wget -q https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz -O google-test-1.12.1.tar.gz)
		message(STATUS "Extracting Google Test...")
		execute_process(COMMAND tar -xf google-test-1.12.1.tar.gz)
	endif()

	# If we want to install our product we do not want to install GTest
	option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" OFF)

	# Ajoute le dossier googletest au build pour réutiliser ses fichiers cmake
	add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-release-1.12.1)

	# Définit les variable pour utiliser googletest dans notre build.
	set(GTEST_FOUND yes)
	set(GTEST_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/deps/googletest-release-1.12.1/googletest/include)
	set(GTEST_LIBRARIES gtest gtest_main)
endif()

######################################################
# Active C++11 pour nos fichier test écrits en C++
set (CMAKE_CXX_STANDARD 11)

######################################################
# add subdirs
add_subdirectory(src)
if (ENABLE_TESTS)
	add_subdirectory(tests)
	add_subdirectory(tests_gtest)
endif (ENABLE_TESTS)
