# TP4 - Partie 2 : Premier pont `NumPy` vers C++

## Objectif
Cette partie introduit le premier vrai passage :

- `numpy.ndarray` 1D
- vers un binding `pybind11`
- puis vers une fonction C++
- puis retour vers `numpy.ndarray`

## Statut pedagogique
Exemple guide.

## Ce qui est fourni
Deux variantes sont proposees :
- [pybind/module/README.md](C:/Users/remlh/Desktop/ProgAvancee/tp4/02_numpy_bridge/pybind/module/README.md)
- [ctypes/module/README.md](C:/Users/remlh/Desktop/ProgAvancee/tp4/02_numpy_bridge/ctypes/module/README.md)

## Fichiers a modifier
Aucun fichier obligatoire.

## Ce que vous devez faire
- lire le code ;
- executer l'exemple ;
- comprendre comment un tableau NumPy 1D est :
  - decrit par `pybind11` ;
  - lu par une fonction C++ ;
  - puis reconstruit pour le retour Python.

## Pourquoi cette partie existe
Cette partie sert de transition entre :
- `01`, ou l'on voit surtout la forme minimale d'un module Python / natif ;
- `03`, ou l'on fera la meme logique sur une image 2D et avec un appel CUDA.

Ici, on apprend donc une seule idee nouvelle :
- faire passer un tableau `NumPy` vers du C++ puis revenir vers `NumPy`.

La variante `pybind11` insiste surtout sur :
- l'API Python exposee par un module compile ;
- `pybind11::array_t<T>` ;
- la lecture de la forme et du pointeur cote C++.

La variante `ctypes` insiste surtout sur :
- `ctypes.CDLL` ;
- `argtypes` et `restype` ;
- les pointeurs explicites vus depuis Python.
