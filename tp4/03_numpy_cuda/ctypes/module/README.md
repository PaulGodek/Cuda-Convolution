# TP4 / Partie 3 / ctypes : Faire un wrapper Python vers CUDA

## Objectif
Dans ce TP, le code CUDA est deja fourni.

Votre travail n'est pas d'ecrire ou de comprendre en profondeur les kernels CUDA. Le vrai objectif est d'apprendre a faire le passage :
- Python
- vers une bibliotheque native
- qui appelle CUDA
- puis qui renvoie le resultat a Python

## Statut pedagogique
Bonus ou comparaison.

## Ce qui est deja fourni
Dans `tp4_ctypes_cuda/student_functions.cu`, les fonctions CUDA sont deja implementees :
- `cudaThresholdDepth` reutilise l'idee du `TP1` ;
- `cudaComputeLargestComponentDiameter` reutilise l'idee du `TP2`.

Deux fonctions C exportees sont egalement fournies :
- `tp4ThresholdDepthBuffer`
- `tp4LargestComponentDiameterBuffer`

Elles constituent l'API native minimale que Python doit appeler.

## Fichiers a modifier
- `tp4_ctypes_cuda/student_binding.py`

## Ce que vous devez faire
Vous devez completer uniquement le wrapper Python dans `tp4_ctypes_cuda/student_binding.py`.

### 1. Charger la bibliotheque native
Completez `_load_library` si necessaire et surtout les signatures `ctypes` :
- `argtypes`
- `restype`

### 2. Envoyer un tableau Python vers CUDA
Completez `threshold_depth(depth, threshold)`.

La fonction doit :
- recevoir un `numpy.ndarray` 2D de type `uint16` ;
- appeler `tp4ThresholdDepthBuffer` ;
- recuperer un masque `uint8` ;
- retourner ce masque a Python sous forme de tableau `numpy`.

### 3. Recuperer un resultat scalaire depuis CUDA
Completez `largest_component_diameter(mask)`.

La fonction doit :
- recevoir un `numpy.ndarray` 2D de type `uint8` ;
- appeler `tp4LargestComponentDiameterBuffer` ;
- retourner un `float`.

## Structure
- `tp4_ctypes_cuda/student_functions.h` : declarations de l'API native ;
- `tp4_ctypes_cuda/student_functions.cu` : implementation CUDA fournie ;
- `tp4_ctypes_cuda/student_binding.py` : wrapper Python a completer ;
- `tp4_ctypes_cuda/__init__.py` : point d'entree du package ;
- `setup.py` et `pyproject.toml` : packaging Python ;
- `../tp4.py` : script d'exemple.

```text
tp4_ctypes_cuda/
  __init__.py
  student_binding.py
  student_functions.h
  student_functions.cu
```

## Installation
Depuis `tp4/03_numpy_cuda/ctypes/module` :

```bash
python -m pip install .
```

## Execution
Depuis `tp4/03_numpy_cuda/ctypes` :

```bash
python tp4.py
```

Les commandes Python sont identiques sous Windows et sous Linux.
Seule la compilation de la bibliotheque native change selon l'OS.
Sous Windows, `tp4.py` essaie automatiquement d'ajouter `%CUDA_PATH%\\bin`
pour que Python puisse trouver les DLL CUDA au moment du chargement de la bibliotheque native.

## Verification attendue
- le package Python s'importe correctement ;
- un tableau `numpy.uint16` peut etre envoye a CUDA ;
- le masque `numpy.uint8` revient correctement dans Python ;
- le diametre revient sous forme de `float` Python ;
- aucun acces disque n'est necessaire pour les appels principaux.
