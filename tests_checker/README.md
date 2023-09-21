Utilisation de mem_checker_shell
=================================

Ce dossier contient les scripts de tests à executer avec la commande `mem_checker_shell`.

Exécution
---------

Vous pouvez simplement exécuter les tests en utilisant la commande :

```sh
./src/mem_checker_shell SCRIPT_DE_TEST
```

Par exemple, nous vous fournissons le script ./tests_checker/tests_basic.check.
Vous pouvez donc lancer

```sh
./src/mem_checker_shell ./tests_checker/tests_basic.check
```

Ecriture de tests
-----------------

Chaque fichier (script de test) contient une suite de cas tests que l'on déclare avec
les commandes :

 * Les lignes de commentaire commencent par `#`
 * Un nouveau test est déclaré avec `Test {NAME}`, le nom pouvant contenir des espaces

On implémente alors un pattern d'allocation/libération avec :

 * Allocation d'un segment avec `Alloc {SIZE}` ou `a {SIZE}`
 * Libération d'un segment alloué avec `Free {ALLOC_ID}` ou `f {ALLOC_ID}`. L'`ALLOC_ID` identifie
 l'allocation par ordre d'appel à `Alloc`. Il est affiché dans la sortie de la commande `Alloc` si
 vous voulez vérifier sa valeur.

Le checker va ensuite comparer l'état de l'allocateur récupérer par `mem_show()`
avec une description de référence fournit dans le script via:

 * `Expect_free_block {SIZE}` ou `F {SIZE}` s'attendent à trouver un bloc libre de la taille {SIZE}.
 * `Expect_busy_block {SIZE}` ou `B {SIZE}` s'attendent à trouver un bloc occupé de la taille {SIZE}.

**Remarques**:
 - Les commandes `Expect_*` calculent automatiquement les offset en sommant les tailles
 utilisées dans les appels précédents.
 - Vous pouvez aussi ignorer l'argument `SIZE` sur les deux commandes pour simplement
vérifier qu'il existe un bloc alloué à cette endroit dans la séquence. Dans ce
cas, les offsets de toutes les opérations suivantes seront aussi ignorés.

On appelle finallement `mem_show` via la commande `Check` qui va parcourir
tous les blocs et les comparer à la description décrite avec les commandes
`Expect_*` plus tôt.

Notez qu'il est possible d'éffectuer plusieurs cycles de vérification dans le même
test car `Check` vide la référence à chaque appel, il faut donc à nouveau appeler
les commandes `Expect_*` pour définir le nouvel état attendu.

Example
-------

```sh
#################################################
# Declare first test
Test Basic single allocation

# Make one allocation
Alloc 10

# Define the expected state
# here, the header takes 24 bytes
# and the memory is 128000 bytes

Expect_busy_block 24
Expect_free_block 127976

# Perform the check
Check

#################################################
# Declare second test
Test Basic single free

#......

#################################################
# Test without checking size, just the sequence
Test Basic not check size

Alloc 10

Expect_busy_block
Expect_free_block

Check
#......
```

Mem_show
--------

Vous pouvez afficher l'état de l'allocateur avec `Mem_show`.

Stop on first
-------------

Par default le checker s'arrête sur le premier test qui échoue. Il est possible
de lui demander de continuer avec l'option `-c` ou `--continue`.

Répétitions
-----------

Les commandes `Expect_*`, `Alloc` et `Free` disposent d'un dernier argument optionnel
qui permet de répéter l'appel plusieurs fois. Pour `Free` cela aura pour effet
d'incrémenter `{ALLOC_ID}` pour chaque répétition.

```sh
Test repeat optional argument

#Make 10 alloc of 8 bytes
Alloc 8 10x
#Free 5 of them in 1 go
Free 1 5x
#Define the final state
Expect_free_block
Expect_busy_block 8 3x
Expect_busy_block 2x
#Perform check
Check
```

Ne vérifier que les premiers blocs
----------------------------------

On peut ne vérifier que les premiers éléments plutôt que faire une liste complète:
```sh
Test check only firsts

#Make 100 alloc of 8 bytes
Alloc 8 100x
#Free the second one
Free 2
#check the first elements
#TODO: define the sizes to be more strict
Expect_busy_block
Expect_free_block
Expect_busy_block
#we do not check the rest
Ignore_next_blocks
#Perform check
Check
```

Implementer des tests plus complexes en C
-----------------------------------------

Vous pouvez aussi regarder `src/tests/test_checker.c` pour implémenter vos tests
directement avec l'API C ce qui peut permettre des patterns plus complexes avec
des boucles par exemples....

Ignorer un bloc
---------------

Suivant votre implémentation vous aurez peut-être des blocs à ignorer dans l'espace mémoire.
Par exemple la structure de d'état global de l'allocateur si vous l'avez placé en début de
zone mémoire.

Dans ce cas vous pouvez ignore l'espace associé en utilisant:

```sh
Expect_skip_block {SIZE} [REPEAT]
S {SIZE} [REPEAT]
```

About the simulator
-------------------

Le checker embarque un simulateur d'allocateur permettant de prédire le comportement
de votre allocateur en fonction de quelques paramêtres à configurer.

Pour l'utiliser on doit tout d'abord définir la configuration de votre allocateur, ceci
doit être fait une fois en début de script via la commande `Sim_set {VAR} {VALUE}`.

On doit ensuite activer le simulateur avec `Sim_enable`

```sh
######## Configure the simulator
# Define the busy block header size
Sim_set BB_SIZE 8
# Define the free block header size
Sim_set FB_SIZE 16
# Declare the space used to store data in front of the memory space.
# Use 0 if you store the allocator state globaly outside of the segment.
Sim_set HEADER_SIZE 0
# Alignement enforced by the allocator
Sim_set ALIGN_SIZE 8
# Size of an address
Sim_set PTR_SIZE 8
# Strategy in use (will also change the one in the allocator)
Sim_set STRATEGY FIRST_FIT
# On free split, make alloc block then the free one (ALLOC_FREE or FREE_ALLOC)
Sim_set SPLIT_STRATEGY ALLOC_FREE

######## Enable the simulator
Sim_enable

######## Do test operations as before
# ...
```

On peut alors faire des opérations et afficher la prédiction de l'allocateur.

```sh
# Configure & enable the simulator
# Sim_set ...
Sim_enable

# Declare a test
Test basic

# Perform ten allocations
Alloc 8 10x
Free 2

# Display the simulator expectation
Sim_show
```
