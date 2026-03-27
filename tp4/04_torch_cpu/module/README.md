# TP4 - Partie 4 : Normalisation CPU sur tenseur PyTorch

## Objectif
Dans cette partie, on ne travaille plus avec `numpy.ndarray`, mais directement avec un `torch.Tensor` sur CPU.
Cette partie n'est pas le coeur du `TP4` : elle sert surtout de transition vers les TPs PyTorch suivants.

## Statut pedagogique
Premiere etape PyTorch.

## Ce qui est fourni
La structure generale du projet est deja en place :
- un dossier `module/` pour le packaging et le build ;
- un dossier `tp4_torch_cpu/` pour le code du module ;
- un script Python d'exemple a la racine de la variante.

Le but est d'apprendre le meme type de pont que dans les parties precedentes,
mais avec des `torch.Tensor` au lieu de tableaux NumPy.

Le module propose maintenant deux fonctions :

```python
threshold(depth: torch.Tensor, threshold: float) -> torch.Tensor
normalize(depth: torch.Tensor) -> torch.Tensor
```

`threshold(...)` sert d'exemple guide :
- on convertit si besoin en `float32` ;
- on lit les valeurs avec un `accessor` ;
- on construit un masque `uint8`.

`normalize(...)` reutilise ensuite les memes idees sur un cas un peu plus riche.

## Fichiers a modifier
- `tp4_torch_cpu/student_functions.cpp`
- `module/setup.py`

## Ce que vous devez faire
Vous devez surtout comprendre deux niveaux :

- `threshold(...)` : exemple simple deja fourni pour apprendre l'API `torch::Tensor` ;
- `normalize(...)` : exercice principal de la partie.

Le binding Python `threshold(...)` / `normalize(...)` est deja fourni.

La fonction principale a completer ici reste :

```python
normalize(depth: torch.Tensor) -> torch.Tensor
```

Le tenseur d'entree :
- est 2D ;
- est de type `torch.uint16` ou `torch.float32` ;
- est sur CPU.

La fonction doit :
- effectuer la normalisation cote C++ ;
- renvoyer un nouveau tenseur `torch.float32` normalise.

Le but est de reconstruire une chaine PyTorch simple avant d'ajouter CUDA dans la partie 5 :
- verification des entrees ;
- choix des conversions utiles ;
- allocation de la sortie ;
- parcours des valeurs ;
- retour du resultat ;
- declaration correcte du build dans `setup.py`.

Dans `setup.py`, la nouveaute principale est :
- `CppExtension(...)` pour declarer une extension PyTorch C++ ;
- `BuildExtension` pour laisser PyTorch gerer le build ;
- une liste de sources C++ seulement, sans fichier `.cu`.

On peut reprendre la normalisation du `TP1` :

```text
out = (in - min) / (max - min)
```

Dans la correction proposee ici :
- l'entree peut etre en `uint16` ou en `float32` ;
- on convertit si besoin en `float32` ;
- la sortie est un nouveau tenseur `float32` dans `[0, 1]`.

Si vous ne savez pas encore comment manipuler un `torch::Tensor` en C++,
commencez par lire `threshold_cpu(...)` dans `student_functions.cpp` :
- il montre la conversion de type ;
- la lecture des dimensions ;
- l'utilisation de `accessor<...>()` ;
- l'ecriture dans un tenseur de sortie.

Le vrai travail restant est donc surtout :
- completer `normalize_cpu(...)` dans `student_functions.cpp` ;
- comprendre comment `module/setup.py` declare une extension PyTorch CPU.

## Structure
- `tp4_torch_cpu/student_binding.cpp`
- `tp4_torch_cpu/student_functions.h`
- `tp4_torch_cpu/student_functions.cpp`
- `tp4_torch_cpu/__init__.py`
- `module/setup.py`
- `module/pyproject.toml`
- `../tp4.py`

```text
tp4_torch_cpu/
  __init__.py
  student_binding.cpp
  student_functions.h
  student_functions.cpp
```

## Installation
Depuis `tp4/04_torch_cpu/module` :

```bash
python -m pip install .
```

Cette commande est identique sous Windows et sous Linux.
Le `setup.py` s'appuie sur `torch.utils.cpp_extension`, qui gere le build
des extensions C++ de PyTorch sur Windows et Linux.

## Execution
Depuis `tp4/04_torch_cpu` :

```bash
python tp4.py
```

## Verification attendue
- `threshold(x, t)` produit un masque CPU `torch.uint8` ;
- `normalize(x)` accepte un tenseur CPU PyTorch ;
- le resultat est aussi un tenseur CPU ;
- les dimensions sont conservees ;
- les valeurs sont normalisees.

## Remarque de build
Cette partie change surtout de bibliotheque Python :
- plus de `ctypes`
- plus de buffer brut `numpy`
- on manipule des `torch::Tensor`

Mais la structure generale reste proche de la partie 3 :
- un dossier `tp4_torch_cpu/` pour le code du module
- un dossier `module/` pour le build et le packaging
- un script Python d'exemple

Cette partie sert de marche intermediaire avant la partie 5 :
- ici, on apprend le pont PyTorch sur CPU ;
- ensuite, on reprendra la meme idee sur GPU avec CUDA.

## Position dans le TP4
Si votre objectif principal est de deployer uniquement le coeur de `TP4`,
vous pouvez considerer cette partie comme une extension pedagogique.
Elle devient en revanche tres utile si vous voulez preparer `TP5`.
