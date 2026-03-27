# TP3 : Normalisation par plan RANSAC et region growing

## Objectif
Dans ce TP, vous partez d'une image de profondeur et d'un masque initial recadre produit par `TP1-bis`.

Le travail est divise en deux parties :
- `TP3` : estimer le plan du fond avec RANSAC et normaliser la profondeur par rapport a ce plan ;
- `TP3.1` : utiliser un fort seuillage sur cette profondeur normalisee puis appliquer un region growing pour obtenir un masque plus propre.

Enfin, vous devez reconstruire le masque pleine taille et calculer la couverture avec les metriques de `TP2`.

## Donnees d'entree
Le programme utilise :
- une image de profondeur complete (`.tiff`, `.tif` ou `.bin`) ;
- un masque initial recadre ;
- un fichier texte de crop ;
- un masque de verite terrain pleine taille.

Par defaut, ces chemins sont definis dans `student_config.h`.

## Travail demande
Vous devez completer uniquement les nouvelles fonctions de `student_functions.cpp`.

Les parties deja traitees dans `TP2` sont considerees comme acquises ici :
- lecture du fichier de crop ;
- recadrage / reconstruction du masque pleine taille ;
- calcul des metriques finales.

Ces fonctions sont deja fournies dans le squelette pour que vous puissiez vous concentrer sur le nouveau traitement.

### 1. TP3 : ajustement d'un plan par RANSAC
Documentation :
- [RANSAC sur Wikipedia](https://fr.wikipedia.org/wiki/RANSAC)
- [RANSAC for planes](https://github.com/kuramin/Ransac_Plane_Detection/blob/master/Ransac_Kuramin.pdf)
Illustration :

![Schema RANSAC](../img/ransac.png)

Completez :
- `fitPlaneRansac`
- `flattenDepthAgainstPlane`

Le plan est ajuste sur les pixels du fond, c'est-a-dire les pixels tels que `mask == 0`.

### 2. TP3.1 : seuillage fort et region growing
Completez :
- `regionGrowMask`

## Fichiers produits
Le programme doit sauvegarder :
- `flattened_depth.bin`
- `flattened_preview.png`
- `region_seed_mask.png`
- `region_grown_mask.png`
- `region_grown_full_mask.png`

## Compilation
Depuis le dossier `tp3` :

Sous Windows :

```bash
g++ -std=c++17 -O2 -o tp3.exe tp3.cpp student_functions.cpp ../io.cpp -lgdiplus
```

Sous Linux :

```bash
g++ -std=c++17 -O2 -o tp3 tp3.cpp student_functions.cpp ../io.cpp
```

## Execution
Avec les chemins par defaut :

```bash
.\tp3.exe
```

Avec tous les parametres :

```bash
.\tp3.exe depth.tiff initial_mask.png crop.txt gt_mask.png 50 8 5 400 20
```
