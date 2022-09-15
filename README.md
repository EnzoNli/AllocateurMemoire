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
- `malloc_stub.h` et `malloc_stub.c` : utilisés pour la génération d'une bibliothèque permettant
  de remplacer la `libc` et de tester votre allocateur avec des programmes existant standard
- des fichiers de test : `test_*.c`
- `Makefile` simple
- des exemples de séquences courtes d'allocations et de libérations `alloc*.in` dans `src/test_shell_sequences`. Vous pouvez les passer en redirigeant l'entrée de votre memshell.

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

