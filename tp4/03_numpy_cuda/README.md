# TP4 - Partie 3 : Wrapper Python vers CUDA avec `numpy`

## Objectif
Cette partie est le coeur du TP.
Elle correspond le plus directement a l'objectif du `TP4` :
exposer depuis Python des fonctions CUDA utiles pour reutiliser l'acceleration.

## Statut pedagogique
Exercice guide.

## Ce qui est fourni
Le code CUDA est deja fourni.

Le travail consiste a construire le passage :
- `numpy.ndarray`
- vers une interface native
- puis vers CUDA
- puis retour vers Python

Deux variantes sont proposees :
- [pybind/module/README.md](C:/Users/remlh/Desktop/ProgAvancee/tp4/03_numpy_cuda/pybind/module/README.md)
- [ctypes/module/README.md](C:/Users/remlh/Desktop/ProgAvancee/tp4/03_numpy_cuda/ctypes/module/README.md)

Ordre recommande :
- commencer par `pybind11`
- garder `ctypes` en bonus ou en comparaison

## Fichiers a modifier
- en priorite : [student_binding.cpp](C:/Users/remlh/Desktop/ProgAvancee/tp4/03_numpy_cuda/pybind/module/tp4_pybind_cuda/student_binding.cpp)
- en bonus : [student_binding.py](C:/Users/remlh/Desktop/ProgAvancee/tp4/03_numpy_cuda/ctypes/module/tp4_ctypes_cuda/student_binding.py)

## Ce que vous devez faire
Les etudiants travaillent surtout sur :
- le passage de tableaux `numpy`
- les signatures natives
- le retour des resultats vers Python

Si vous ne deviez garder qu'une seule vraie partie de `TP4` pour le deploiement,
c'est celle-ci.

La variante `pybind11` insiste sur :
- l'integration C++/Python
- les objets `numpy`
- une API Python plus naturelle

La variante `ctypes` insiste sur :
- les signatures C
- les pointeurs
- les buffers explicites
