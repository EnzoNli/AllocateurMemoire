Tests avec Google Test
======================

Lorsque vous devez écrire des tests (unitaires) il est bien plus pratique
d'utiliser un framework pour :

 * Organiser ses tests
 * Les lancer
 * Faire le reporting.
 * Aider au debuggage.
 * Lancer un seul des tests

Dans le monde du C/C++ l'un des très bon framework du moment est Google Test
(https://github.com/google/googletest), bien que d'autres existent.

Include & compilation
---------------------

Vous devez inclure :

```c++
#include <gest/gest.h>
```

Et linker votre programme avec :

```sh
# si gtest est dans vote système
g++ --std=c++11 -lgtest_main -lgtest -lpthread .....

# Si vous devez préciser où le trouver
# Remplacer {GTEST_PREFIX} par le chemin correspondant.
g++ --std=c++11 -lgtest_main -lgtest -lpthread -I{GTEST_PREFIX}/include -L{GTEST_PREFIX}/lib .....
```

Exemple basic de test
---------------------

Un test se déclare avec la macro `TEST()` qui attend:

 * Un nom de groupe (*test case*)
 * Le nom du test (*test name*)

```c++
TEST(unit, mem_space_get_addr)
{
    //.... code du test ...
}
```

On décrit alors ses tests en code normal pour appeler ses fonctions.
Afin de vérifier certaines propriétés (valeurs de variables...) on utilisera
des `ssertion` avec les mots clés: 

 * `ASSERT_EQ(variable, expected_value)` : Test si `variable` vaut `expected_value`.
   Si NON, alors le test échoue, s'interromp et des informations de debug seront affichées.
*  `EXPECT_EQ(variable, expected_value)` : Test si `variable` vaut `expected_value`.
   Si NON, alors le test est marqué en échec et des informations de debug seront affichées.
   Sur `EXPECT` le test continue son chemin pour afficher plus d'information.

Exemple simple:

```c++
TEST(unit, mem_space_get_addr)
{
    int a = 10;
    ASSERT_EQ(a, 10); // OK => continue
    ASSERT_EQ(a, 11);// FAIL => stop & affiche "a != 10"
}
```

Le framework fournis toute une liste de macros de test:

 * `ASSERT_EQ`, `ASSERT_NE`, `ASSERT_LT`....
 * `EXPECT_EQ`, `EXPECT_NE`, `EXPECT_LT`....

Dont on trouvera la liste et documentation ici :
http://google.github.io/googletest/reference/assertions.html

Remarque : C++
--------------

Une remarque, le framework est écrit en C++ bien que son usage se fait
pour l'essentiel avec des notations type C.

Si on test du code en C (notre cas), il faut donc bien veiller à le
préciser dans la gestion des en-tête avec :

```cpp
extern "C" {
    #include "my_c_header.h"
}
```

A moins que `extern "C"` ne soit déjà présent dans le `.h` lui même.

Subtilité 1 : NULL -> nullptr
-----------------------------

Deux subtilités à noter et dont vous aurez peut-être besoin pour ce TP :

```c++
// Ne pas écrire
// EXPECT_EQ(ptr, NULL)

// Mais :
EXPECT_EQ(ptr, nullptr)
```

Ceci parce que sans cela le compilateur ne sais pas faire la différence entre
`NULL` et `0` qui ici change la façon dont `EXPECT_EQ` doit réagit en interne.

Subtilité 2 : 0ul
-----------------

Pour ce TP vous allez souvent utiliser les nombre de type `size_t`, donc `long`.

Donc si vous voulez éciter des **warnings** ou **erreurs** suivant le compilateur
utilisé, il est préférable de préciser le type de ses valeurs fixes :

```c++
size_t value = 100;

// Ne pas écrire
// EXPECT_EQ(value, 100)

// Mais : (ul => unsigned long)
EXPECT_EQ(value, 100ul)
```

GDB break sur erreur
--------------------

Avec Google test, si vous voulez arreter GDB sur une ligne d'échec d'un test
vous pouvez lancer le test manuellement avec l'option :

```sh
gdb ./tests_gtest/test_unit --gtest_break_on_failure
```

Vous pouvez sélection un sous test en utilisant :

```sh
./tests_gtest/test_unit --gtest_filter=unit.mem_space_get_size
```

Pour d'autres projets : compilation de google test
--------------------------------------------------

Google test peut se compiler simplement à la main si
vous ne l'avez pas dans votre système :

Pour le TP, notre `Makefile` s'en charge pour vous, voir la cible `googletest`.

Sinon :

```sh
# Téléchargement de l'archive :
wget https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz -O google-test-1.12.1.tar.gz
# extraction
tar -xvf google-test-1.12.1.tar.gz
# compilation et installation dans un dossize de son home:
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/usr-googletest
make
make install
```
