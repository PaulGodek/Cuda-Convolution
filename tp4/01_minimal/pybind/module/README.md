# TP4 - Partie 1B : Exemple minimal avec `pybind11`

## Objectif
Decouvrir deux chemins natifs avec `pybind11` :
- une fonction pure C++
- une fonction qui appelle CUDA

Le module expose deux fonctions :

```python
add_ints_cpp(a: int, b: int) -> int
add_ints_cuda(a: int, b: int) -> int
```

## Statut pedagogique
Exemple a lire et a executer.

## Ce qui est fourni
Un module minimal complet avec :
- un chemin purement C++ ;
- un chemin qui appelle CUDA.

## Fichiers a modifier
Aucun fichier obligatoire.

## Ce que vous devez faire
- lire le code ;
- lancer l'exemple ;
- reperer la separation entre binding Python et code natif.

## Structure
- `mini_pybind/bindings.cpp` : bindings Python
- `mini_pybind/cuda_add.cu` : addition minimale sur CUDA
- `mini_pybind/__init__.py` : point d'entree du module Python
- `setup.py` : build minimal du module
- `pyproject.toml` : dependances de build
- `../demo.py` : exemple Python

```text
mini_pybind/
  __init__.py
  bindings.cpp
  cuda_add.cu
```

## Installation
Depuis `module` :

```bash
python -m pip install .
```

## Execution
Depuis le dossier `pybind` :

```bash
python demo.py
```

Sur Windows, `demo.py` essaie automatiquement d'ajouter le dossier
`%CUDA_PATH%\\bin` pour que Python puisse trouver les DLL CUDA au moment
de l'import du module natif.

## Verification attendue
- les deux fonctions Python s'importent ;
- `add_ints_cpp` renvoie la bonne somme ;
- `add_ints_cuda` renvoie la meme somme.

## Remarque de build
Cet exemple est volontairement minimal.
Le `setup.py` essaie de gerer :
- Windows avec `nvcc` + MSVC ;
- Linux avec `nvcc` + `c++`.

Selon la machine, compiler un fichier `.cu` depuis `setuptools`
peut quand meme demander une petite adaptation supplementaire de la toolchain.
