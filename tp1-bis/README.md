# TP1-bis : Suppression simple du fond à partir d'une carte de profondeur

## Objectif
Dans ce TP, vous allez construire une petite chaîne de traitement d'image en C++ pour isoler le premier plan à partir d'une image de profondeur.

Le but est de produire :
- un masque binaire du premier plan
- une image de profondeur nettoyée
- un fichier texte décrivant le recadrage appliqué

Ce TP ne demande ni CUDA ni OpenCV.

## Données d'entrée
Le programme travaille à partir d'une image de profondeur :
- au format `.tiff` ou `.tif`
- ou au format binaire maison du projet

Le chemin par défaut de l'image est défini dans `student_config.h`.

## Travail demandé
Vous devez compléter les fonctions dans `student_functions.cpp`.

### 1. Construction du masque
Vous commencerez par compléter `thresholdDepth` pour créer le masque binaire, puis `erodeMask`, `dilateMask`, `openMask` et `closeMask` pour nettoyer ce masque.

### 2. Prétraitement de la profondeur
Vous compléterez ensuite `normalizeDepth` et `invertDepth` afin de transformer correctement l'image de profondeur.

### 3. Application du masque et recadrage
Enfin, vous compléterez maskDepth, cropMask et cropDepth pour conserver les pixels dont le masque vaut 255, puis recadrer le masque et l’image de profondeur obtenus.

## Chaîne de traitement demandée
Le programme principal applique les étapes suivantes :

1. charger une image de profondeur
2. seuiller la profondeur brute pour obtenir un masque binaire
3. nettoyer ce masque par morphologie binaire
4. appliquer le masque à l'image de profondeur
5. recadrer le masque et l'image de profondeur
6. normaliser la profondeur recadrée sur `[0, 65535]`
7. inverser la profondeur
8. sauvegarder les résultats

## Ce qu'il faut comprendre
### Seuillage
Le seuillage produit un masque binaire 8 bits :

```text
mask[i] = 255 si 0 <= depth[i] <= threshold, sinon 0
```

Le seuil est appliqué avant la normalisation.

### Morphologie binaire
On utilise un élément structurant carré `3x3`.

- `erodeMask` conserve un pixel blanc seulement si les 9 pixels du voisinage valent `255`
- `dilateMask` met un pixel blanc si au moins un pixel du voisinage vaut `255`
- les bords sont considérés comme du fond

### Application du masque
```text
output[i] = depth[i] si mask[i] != 0, sinon 0
```

### Recadrage
Le recadrage est piloté par `student_config.h` :
- `cropX`
- `cropY`
- `cropW`
- `cropH`

Ces paramètres peuvent être modifiés pour adapter le TP au jeu de données.

## Fichiers produits
Le programme doit produire :
- `mask_final.png`
- `foreground_depth.bin`
- `mask_final_crop.txt`

Le fichier `mask_final_crop.txt` contient :
- `originalWidth`
- `originalHeight`
- `cropX`
- `cropY`
- `cropW`
- `cropH`

## Compilation
Depuis le dossier `tp1-bis` :

```bash
g++ -std=c++17 -O2 -o tp1_bis tp1.cpp student_functions.cpp ../io.cpp
```

Sous Windows :

```bash
g++ -std=c++17 -O2 -o tp1_bis.exe tp1.cpp student_functions.cpp ../io.cpp -lgdiplus
```

## Exécution
Avec l'image par défaut et le seuil par défaut :

```bash
.\tp1_bis.exe
```

Avec une image explicite :

```bash
.\tp1_bis.exe depth_image.tiff
```

Avec une image et un seuil :

```bash
.\tp1_bis.exe depth_image.tiff 345
```

Avec seulement un seuil :

```bash
.\tp1_bis.exe 345
```

## Vérifications attendues
- `thresholdDepth` doit produire une image binaire `0/255`
- `openMask` doit retirer les petits bruits isolés
- `closeMask` doit reboucher de petits trous
- `cropMask` et `cropDepth` doivent utiliser les paramètres de `student_config.h`
- `foreground_depth.bin` doit correspondre a l´image recadré puis traité
- `mask_final.png` doit mettre visuellement en évidence l´image recadré avec le masque
- `mask_final_crop.txt` doit décrire correctement le recadrage appliqué

## Fichiers
- `tp1.cpp` : programme principal
- `student_functions.h` : déclarations
- `student_functions.cpp` : fonctions à compléter
- `student_config.h` : paramètres modifiables

## Durée
2h
