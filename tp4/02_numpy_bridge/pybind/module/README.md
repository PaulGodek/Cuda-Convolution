# TP4 - Partie 2 / pybind11 : Pont `NumPy 1D`

## Objectif
Apprendre le premier vrai passage :
- `numpy.ndarray` 1D
- vers un binding `pybind11`
- puis vers une fonction C++
- puis retour vers `numpy.ndarray`

Le module expose une seule fonction :

```python
threshold_vector(values: np.ndarray[np.uint16], threshold: int) -> np.ndarray[np.uint8]
```

## Statut pedagogique
Exemple guide a lire et a executer.

## Ce qui est fourni
Un module minimal complet avec :
- un mini-exemple `NumPy` 1D sans CUDA pour apprendre le passage `pybind11`.

## Fichiers a modifier
Aucun fichier obligatoire.

## Ce que vous devez faire
- lire le code ;
- lancer l'exemple ;
- reperer la separation entre binding Python et code natif ;
- identifier ou `pybind11` stocke :
  - la taille du vecteur ;
  - le pointeur vers les donnees ;
  - le tableau NumPy de sortie.

## Structure
- `tp4_pybind_bridge/bindings.cpp` : bindings Python
- `tp4_pybind_bridge/vector_threshold.cpp` : traitement C++ simple sur un vecteur 1D
- `tp4_pybind_bridge/__init__.py` : point d'entree du module Python
- `setup.py` : build minimal du module
- `pyproject.toml` : dependances de build
- `../demo.py` : exemple Python

```text
tp4_pybind_bridge/
  __init__.py
  bindings.cpp
  vector_threshold.cpp
  vector_threshold.h
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

## Verification attendue
- la fonction Python s'importe ;
- `threshold_vector` montre clairement :
  - `pybind11::array_t<T>`
  - `request()`
  - `shape[0]`
  - `ptr`
  - la creation d'un nouveau tableau NumPy de sortie
  - le remplissage explicite de cette sortie cote C++.

## Pont vers la partie 3
Le mini-exemple `threshold_vector(...)` sert de transition vers `TP4/03`.

La logique generale est la meme qu'en partie 3, mais avec :
- un vecteur 1D au lieu d'une image 2D ;
- du C++ simple au lieu d'un appel CUDA.
