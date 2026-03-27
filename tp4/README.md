# TP4 : Faire le pont entre CUDA et Python

L'objectif central de `TP4`, dans la logique du projet, est d'exposer depuis Python
des fonctions natives / CUDA utiles pour reutiliser l'acceleration introduite auparavant.

La progression ci-dessous sert a rendre ce passage lisible et progressif.

Le `TP4` est organise comme une progression en autonomie.

L'objectif n'est pas seulement de changer de bibliotheque Python, mais surtout de passer :
- d'un exemple a lire ;
- a un premier pont `NumPy` vers C++ ;
- a un exercice guide ;
- puis a un exercice plus autonome.

## Partie 1 : exemple a lire et executer
[01_minimal/README.md](C:/Users/remlh/Desktop/ProgAvancee/tp4/01_minimal/README.md)

Cette partie est une reference.
Elle sert a observer la structure generale d'un projet Python -> natif -> CUDA.
Dans chaque sous-partie, l'installation du module se fait avec `python -m pip install .`.

## Partie 2 : premier pont `NumPy`
[02_numpy_bridge/README.md](C:/Users/remlh/Desktop/ProgAvancee/tp4/02_numpy_bridge/README.md)

Cette partie introduit un premier passage explicite :
- `numpy.ndarray` 1D
- vers du C++
- puis retour vers `numpy.ndarray`

Le but est de preparer la partie 3 sans introduire tout de suite le 2D et CUDA.
Deux variantes sont proposees :
- `pybind11` pour voir le module compile cote Python ;
- `ctypes` pour voir le meme pont avec des signatures natives explicites.
L'installation attendue passe aussi par `python -m pip install .`.

## Partie 3 : exercice guide
[03_numpy_cuda/README.md](C:/Users/remlh/Desktop/ProgAvancee/tp4/03_numpy_cuda/README.md)

C'est la partie principale du TP.
C'est elle qui colle le plus directement a l'objectif de `README_private.md` :
faire le pont entre les fonctions CUDA et un usage Python.

Ordre recommande :
- `pybind11` en premier
- `ctypes` en bonus ou en comparaison

Ici, le code CUDA est fourni et le travail porte surtout sur :
- le binding ;
- le packaging ;
- le passage de `numpy.ndarray` vers le code natif.
L'installation attendue passe aussi par `python -m pip install .`.

## Partie 4 : premier pont PyTorch sur CPU
[04_torch_cpu/module/README.md](C:/Users/remlh/Desktop/ProgAvancee/tp4/04_torch_cpu/module/README.md)

Cette partie introduit `torch.Tensor` sans encore ajouter CUDA.
Le but est de comprendre :
- comment recevoir un tenseur PyTorch ;
- comment travailler dessus dans une extension C++ ;
- comment renvoyer un tenseur PyTorch.
Cette partie sert surtout de transition vers `TP5`.
L'installation attendue passe aussi par `python -m pip install .`.

## Partie 5 : version PyTorch CUDA plus autonome
[05_torch_cuda/module/README.md](C:/Users/remlh/Desktop/ProgAvancee/tp4/05_torch_cuda/module/README.md)

Cette partie reprend la meme famille d'idees, mais avec :
- l'interface PyTorch sur GPU ;
- un kernel CUDA ;
- plus de responsabilite sur le code natif et le build.
L'installation attendue passe aussi par `python -m pip install .`.

## Chemin recommande de deploiement
Pour un deploiement simple et robuste du TP :
- coeur recommande : `01` -> `02` -> `03`
- extension PyTorch testable sans GPU PyTorch : `04`
- extension PyTorch CUDA : `05` uniquement sur des machines avec une build CUDA de `torch`

Autrement dit :
- si vous voulez deployer le coeur du TP4, la partie indispensable est `03`
- `04` et `05` servent a ouvrir progressivement vers les TPs suivants

## Prerequis
- `01` et `03` demandent une toolchain C++ / CUDA locale
- `04` demande une installation PyTorch, meme CPU-only
- `05` demande une installation PyTorch avec support CUDA

## Regle de nommage
Dans les parties 3, 4 et 5 :
- `student_binding.*` designe la couche de binding / interface ;
- `student_functions.*` designe la logique native / CUDA.
