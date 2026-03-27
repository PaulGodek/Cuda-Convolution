# TP4 / Partie 3 / pybind11 : Faire un wrapper Python vers CUDA

## Objectif
Dans cette variante, le code CUDA est deja fourni.

Le travail demande porte sur le binding Python :
- recevoir un tableau `numpy` en Python ;
- le transmettre a du code natif ;
- appeler CUDA ;
- reconstruire un objet Python propre au retour.

## Statut pedagogique
Exercice guide principal.

## Ce qui est deja fourni
Dans `tp4_pybind_cuda/student_functions.cu`, les fonctions CUDA sont deja implementees :
- `cudaThresholdDepth` ;
- `cudaComputeLargestComponentDiameter`.

Deux helpers C++ sont egalement fournis :
- `thresholdDepthBuffer`
- `largestComponentDiameterBuffer`

Ils prennent des buffers en memoire et constituent l'interface que votre binding `pybind11` doit appeler.

## Fichiers a modifier
- `tp4_pybind_cuda/student_binding.cpp`

## Ce que vous devez faire
Vous devez completer `tp4_pybind_cuda/student_binding.cpp`.

Le role des fichiers est le suivant :
- `tp4_pybind_cuda/student_functions.cu` contient la logique C++/CUDA ;
- `tp4_pybind_cuda/student_binding.cpp` fait seulement le passage `numpy <-> C++` avec `pybind11` ;
- `tp4_pybind_cuda/__init__.py` reexporte proprement l'API Python.

Avant cette partie, vous pouvez vous appuyer sur `TP4/02_numpy_bridge/pybind` :
- `threshold_vector(...)` montre deja la logique `numpy -> pybind11 -> C++ -> numpy` ;
- ici, on reprend la meme idee, mais avec une image 2D au lieu d'un vecteur 1D.

### 1. Exposer un tableau `numpy.uint16` vers CUDA
Completez `threshold_depth(depth, threshold)`.

La fonction Python doit :
- recevoir un `numpy.ndarray` 2D de type `uint16` ;
- appeler `thresholdDepthBuffer` ;
- retourner un nouveau `numpy.ndarray` 2D de type `uint8`.

### 2. Exposer un tableau `numpy.uint8` vers CUDA
Completez `largest_component_diameter(mask)`.

La fonction Python doit :
- recevoir un `numpy.ndarray` 2D de type `uint8` ;
- appeler `largestComponentDiameterBuffer` ;
- retourner un `float`.

### 3. Exposer les fonctions dans le module Python
Completez aussi le bloc `PYBIND11_MODULE(...)`.

Ici, le travail est plus court :
- reprendre la logique deja vue en partie 1 avec `m.def(...)` ;
- exposer `threshold_depth(...)` ;
- exposer `largest_component_diameter(...)`.

## Structure
- `tp4_pybind_cuda/student_functions.h` : declarations C++ ;
- `tp4_pybind_cuda/student_functions.cu` : implementation CUDA fournie ;
- `tp4_pybind_cuda/student_binding.cpp` : binding `pybind11` a completer ;
- `setup.py` : build du package Python et du module natif ;
- `tp4_pybind_cuda/__init__.py` : point d'entree du package ;
- `pyproject.toml` : configuration de build ;
- `../tp4.py` : script d'exemple lance depuis la racine `pybind/`.

```text
tp4_pybind_cuda/
  __init__.py
  student_binding.cpp
  student_functions.h
  student_functions.cu
```

## Installation
Depuis `tp4/03_numpy_cuda/pybind/module` :

```bash
python -m pip install .
```

## Execution
Depuis `tp4/03_numpy_cuda/pybind` :

```bash
python tp4.py
```

Les commandes Python sont identiques sous Windows et sous Linux.
Sous Windows, `tp4.py` essaie automatiquement d'ajouter `%CUDA_PATH%\\bin`
pour que Python puisse trouver les DLL CUDA au moment de l'import du module natif.

Le `setup.py` essaie de gerer :
- Windows avec `nvcc` + MSVC ;
- Linux avec `nvcc` + `c++`.

## Verification attendue
- le module Python s'importe sans `ctypes` ;
- un tableau `numpy.uint16` part bien vers le code natif ;
- le masque `numpy.uint8` revient correctement dans Python ;
- le diametre revient sous forme de `float` ;
- aucun acces disque n'est necessaire pour l'API principale.
