# TP4 - Partie 2 / ctypes : Pont `NumPy 1D`

## Objectif
Apprendre le premier vrai passage :
- `numpy.ndarray` 1D
- vers une bibliotheque native chargee avec `ctypes`
- puis retour vers `numpy.ndarray`

Le module expose une seule fonction :

```python
threshold_vector(values: np.ndarray[np.uint16], threshold: int) -> np.ndarray[np.uint8]
```

## Statut pedagogique
Exemple guide a lire et a executer.

## Ce qui est fourni
Un module minimal complet avec :
- un petit package Python `tp4_ctypes_bridge/` ;
- une bibliotheque native C++ simple sans CUDA ;
- un script Python d'exemple dans `../demo.py`.

## Fichiers a modifier
Aucun fichier obligatoire.

## Ce que vous devez faire
- lire le code ;
- compiler la bibliotheque native ;
- lancer l'exemple ;
- reperer la separation entre :
  - le wrapper Python `ctypes` ;
  - la fonction native exportee ;
  - le tableau NumPy de sortie.

## Structure
- `tp4_ctypes_bridge/student_binding.py` : wrapper Python avec `ctypes`
- `tp4_ctypes_bridge/vector_threshold_lib.h` : API native exportee
- `tp4_ctypes_bridge/vector_threshold_lib.cpp` : traitement C++ simple sur un vecteur 1D
- `tp4_ctypes_bridge/__init__.py` : point d'entree du module Python
- `setup.py` : packaging Python du module
- `pyproject.toml` : dependances de build
- `../demo.py` : exemple Python

```text
tp4_ctypes_bridge/
  __init__.py
  student_binding.py
  vector_threshold_lib.h
  vector_threshold_lib.cpp
```

## Installation
Depuis `module` :

```bash
python -m pip install .
```

## Execution
Depuis le dossier `ctypes` :

### Compilation native sous Windows avec `cl`
```bash
cl /LD module\\tp4_ctypes_bridge\\vector_threshold_lib.cpp /Femodule\\tp4_ctypes_bridge\\tp4_ctypes_bridge.dll
```

### Compilation native sous Linux avec `g++`
```bash
g++ -shared -fPIC module/tp4_ctypes_bridge/vector_threshold_lib.cpp -o module/tp4_ctypes_bridge/libtp4_ctypes_bridge.so
```

### Lancement Python
```bash
python demo.py
```

## Verification attendue
- le package Python s'importe ;
- la bibliotheque native est chargee par `ctypes` ;
- `threshold_vector(...)` montre clairement :
  - la verification du type `numpy.uint16`
  - le passage d'un pointeur brut vers le code natif
  - la creation du tableau NumPy de sortie.

## Pont vers la partie 3
Le mini-exemple `threshold_vector(...)` sert de transition vers `TP4/03`.

La logique generale est la meme qu'en partie 3, mais avec :
- un vecteur 1D au lieu d'une image 2D ;
- du C++ simple au lieu d'un appel CUDA.

Cette variante insiste surtout sur :
- `ctypes.CDLL`
- `argtypes` et `restype`
- les pointeurs explicites
- les buffers NumPy vus depuis Python.
