# TP4 - Partie 1A : Exemple minimal avec `ctypes`

## Objectif
Montrer le principe d'un wrapper Python sans CUDA.

## Statut pedagogique
Exemple a lire et a executer.

## Ce qui est fourni
Une fonction C minimale et un petit script Python qui la charge avec `ctypes`.

Le code natif exporte une fonction C tres simple :

```c
int add_ints(int a, int b);
```

## Fichiers a modifier
Aucun fichier obligatoire.

## Ce que vous devez faire
- lire le code ;
- compiler la bibliotheque native ;
- lancer le script Python ;
- comprendre le chargement de la fonction native.

## Structure
- `native_module/native_lib.c` : bibliotheque C minimale
- `demo.py` : exemple Python

```text
native_module/
  native_lib.c
```

## Installation
Il n'y a pas de package Python a installer dans cet exemple.

## Execution
### Compilation native sous Windows avec `cl`
```bash
cl /LD native_module\\native_lib.c /Fenative_lib.dll
```

### Compilation native sous Linux avec `gcc`
```bash
gcc -shared -fPIC native_module/native_lib.c -o libnative_lib.so
```

### Lancement Python
```bash
python demo.py
```

## Verification attendue
- la bibliotheque native est chargee ;
- `add_ints(4, 7)` renvoie `11`.
