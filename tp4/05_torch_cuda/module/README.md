# TP4 - Partie 5 : Normalisation CUDA sur tenseur PyTorch

## Objectif
Dans cette derniere partie, on ne travaille plus avec `numpy.ndarray`, mais directement avec un `torch.Tensor` sur GPU.
Cette partie pousse plus loin l'idee du `TP4`, mais elle depend d'un environnement PyTorch CUDA complet.

## Statut pedagogique
Exercice plus autonome.

## Ce qui est fourni
La structure generale du projet est deja en place :
- un dossier `module/` pour le packaging et le build ;
- un dossier `tp4_torch_cuda/` pour le code du module ;
- un script Python d'exemple a la racine de la variante.

Le but est de reconstruire vous-memes une plus grande partie de la chaine
qu'en partie 4 CPU.

## Fichiers a modifier
- `tp4_torch_cuda/student_functions.cu`
- `module/setup.py`

## Ce que vous devez faire
Vous devez exposer une fonction Python :

```python
normalize(depth: torch.Tensor) -> torch.Tensor
```

Le tenseur d'entree :
- est 2D ;
- est de type `torch.uint16` ou `torch.float32` ;
- est deja sur le GPU.

La fonction doit :
- lancer un kernel CUDA ;
- renvoyer un nouveau tenseur `torch.float32` normalise.

Par rapport a la partie 4 CPU, cette partie est moins guidee.
Le but est que vous reconstruisiez vous-memes une plus grande partie de la chaine :
- choix des conversions utiles ;
- allocation de la sortie ;
- lancement du kernel ;
- retour du resultat ;
- declaration correcte du build dans `setup.py`.

Le binding Python `normalize(...)` est deja fourni.

Le vrai travail restant porte surtout sur :
- `normalize_cuda(...)` dans `student_functions.cu` ;
- la declaration correcte de l'extension CUDA dans `module/setup.py`.

On peut reprendre la normalisation du `TP1` :

```text
out = (in - min) / (max - min)
```

Dans la correction proposee ici :
- l'entree peut etre en `uint16` ou en `float32` ;
- on convertit si besoin en `float32` ;
- la sortie est un nouveau tenseur `float32` dans `[0, 1]`.

## Structure
- `tp4_torch_cuda/student_binding.cpp`
- `tp4_torch_cuda/student_functions.h`
- `tp4_torch_cuda/student_functions.cu`
- `tp4_torch_cuda/__init__.py`
- `module/setup.py`
- `module/pyproject.toml`
- `../tp4.py`

```text
tp4_torch_cuda/
  __init__.py
  student_binding.cpp
  student_functions.h
  student_functions.cu
```

## Installation
Depuis `tp4/05_torch_cuda/module` :

```bash
python -m pip install .
```

Cette commande est identique sous Windows et sous Linux.
Le `setup.py` s'appuie sur `torch.utils.cpp_extension`, qui gere le build
des extensions C++ / CUDA pour PyTorch sur Windows et Linux.
Il faut en revanche une installation de PyTorch avec support CUDA :
une version CPU-only de `torch` ne suffit pas pour cette partie.

Si la machine des etudiants ne dispose pas d'une build CUDA de PyTorch,
il est raisonnable de deployer `TP4` sans cette partie.

## Execution
Depuis `tp4/05_torch_cuda` :

```bash
python tp4.py
```

Sous Windows, `tp4.py` essaie automatiquement d'ajouter `%CUDA_PATH%\\bin`
pour que Python puisse trouver les DLL CUDA au moment de l'import du module natif.

## Verification attendue
- `normalize(x)` accepte un tenseur CUDA PyTorch ;
- le resultat est aussi un tenseur CUDA ;
- les dimensions sont conservees ;
- les valeurs sont normalisees.

## Remarque de build
Cette partie change surtout de bibliotheque Python :
- plus de `ctypes`
- plus de buffer brut `numpy`
- on manipule des `torch::Tensor`

Mais la structure generale reste proche de la partie 4 CPU :
- un dossier `tp4_torch_cuda/` pour le code du module
- un dossier `module/` pour le build et le packaging
- un script Python d'exemple

La difference principale est le niveau d'autonomie demande :
- en partie 4 CPU, on apprend d'abord le pont PyTorch sur CPU ;
- en partie 5, on reprend la meme idee sur GPU avec CUDA.

## Position dans le TP4
Cette partie est une extension avancee.
Le coeur deployable du `TP4` reste la partie 3, et la partie 4 CPU permet
de preparer cette version CUDA si l'environnement le permet.
