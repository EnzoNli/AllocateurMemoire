Contenu de l'archive pour le TP Allocateur mémoire.
===================================================

Cette archive contient :
*  le sujet du TP : `TD-TP-Memoire.pdf`
*  les sources d'un squelette de votre allocateur : `src/`
*  des tests pour votre allocateur : `src/tests/`
*  des exemples d'entrée pour mem_shell dans : `src/test_shell_sequences`

Contenu de src/
---------------

- `mem.h` et `mem_os.h` : l'interface de votre allocateur. 
  `mem.h` définit les fonctions utilisateur (`mem_alloc`, `mem_free`), 
   alors que `mem_os.h` définit les fonctions définissant la stratégie d'allocation
- `mem_space.h` et `mem_space.c` : définissent la mémoire à gérer et des fonctions utilitaires pour connaître sa taille et son adresse de début.
  Ces fonctions seront utilisées dans la fonction `mem_init`.
- `mem_shell.c` ; un interpreteur simple de commandes d'allocation et de libération 
   vous permettant de tester votre allocateur de manière intéractive
-`mem.c` : le squelette de votre allocateur, le fichier que vous devez modifier.
- `mem_malloc_stub.h` et `mem_malloc_stub.c` : utilisés pour la génération d'une bibliothèque permettant
  de remplacer la `libc` et de tester votre allocateur avec des programmes existant standard
- des fichiers de test : `test_*.c`
- `Makefile` simple
- des exemples de séquences courtes d'allocations et de libérations `alloc*.in` dans `src/test_shell_sequences`. Vous pouvez les passer en redirigeant l'entrée de votre memshell.

Compilation des tests
---------------------

Les tests utilisent le framework Google Test (https://github.com/google/googletest) qui est un framework de test unitaire.

Vous trouverez la documentation du framework ici : https://google.github.io/googletest/primer.html.

Une version a été installé sur les machine de l'université dans le dossier : `XXXXXX_TODO_XXXX` et pointé
par le makefile.

Si vous travaillez sur votre propre installation vous devez compiler et installer la librairire avant usage.

Soit en utilisant le makefile fournit :

```sh
make googletest
```

Compilation
-----------

Le projet se compile simplement en utilisant la commande:

```sh
make
```

On peu tourner les tests en faisant (s'arrête au premier échec):

```sh
make test
```

Execute tous les tests et affiche un résumé :

```sh
make test_all
```

On peut nettoyer en faisant:

```sh
make clean
```

On peu générer une archive des sources avec :

```sh
make archive
```

--------------------

Avec Google test, si vous voulez arreter GDB sur une ligne d'échec d'un test
vous pouvez lancer le test manuellement avec l'option :

```sh
gdb ./tests/test_unit --gtest_break_on_failure
```

Vous pouvez sélection un sous test en utilisant :

```sh
./tests/test_unit --gtest_filter=unit.mem_space_get_size
```

Implémenter vos propres tests
-----------------------------

Vous pouvez étendez les exemples de tests unitiaire présents dans
`src/tests/test_unit.cpp` et `src/tests/test_basic.cpp`.

Pour d'autres projets : compilation de google test
--------------------------------------------------

Google test peut se compiler simplement à la main :

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
