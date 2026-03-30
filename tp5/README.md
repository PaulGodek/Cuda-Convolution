# TP5 : Squelette, graphe et courbure globale

## Objectif
Dans ce TP, vous partez d'un masque de vérité terrain déjà segmenté.

L'objectif est de calculer un squelette sur chaque composante connexe, puis d'utiliser le squelette comme un graphe pour extraire une ligne principale et mesurer une courbure globale avec une géométrie simple basée sur la corde et la sagitta.

Le but n'est donc plus de segmenter l'objet, mais d'analyser sa forme.

## Données d'entrée
Le programme utilise une seule image de masque GT PNG.

Par défaut, ce chemin est défini dans `student_config.h`.

Le masque est interprété comme suit :
- pixels non nuls = objet ;
- pixels nuls = fond.

## Travail demandé
Vous devez compléter uniquement les fonctions de `student_functions.cpp`.

Les parties suivantes sont déjà fournies dans `provided_functions.cpp` :
- binarisation du masque ;
- extraction des composantes connexes ;
- recadrage d'une composante ;
- skeletonization de Zhang-Suen ;
- voisinage actif du squelette avec `skeletonNeighbors` ;
- reconstruction d'un chemin avec `reconstructPath` ;
- helpers géométriques (`pointLineDistance`, `circleRadiusFromThreePoints`) ;
- visualisation finale.

Le plus simple est d'avancer dans l'ordre suivant.

### 1. Lire le squelette comme un graphe
Complétez :
- `traceBranch`

Le squelette est interprété comme un graphe implicite non pondéré :
- un pixel blanc du squelette = un noeud ;
- deux pixels voisins 8-connexes = une arête ;
- chaque arete compte pour un pas.

On utilisera les degrés suivants :
- degré 1 = extrémité ;
- degré 2 = point régulier ;
- degré >= 3 = jonction.

`traceBranch` doit suivre une branche depuis une extrémité jusqu'à une jonction ou une fin.
Le plus simple est d'utiliser `skeletonNeighbors`, puis de retirer le pixel précédent pour savoir dans quelle direction continuer.

### 2. Nettoyer le squelette
Complétez :
- `pruneTerminalBranches`

Cette fonction doit supprimer les petites branches terminales parasites afin de simplifier le squelette avant l'analyse géométrique.
On ne supprime ici qu'une petite branche terminale qui se rattache à une jonction.

### 3. Extraire une ligne principale
Complétez :
- `shortestPathLengths`
- `extractMainPath`

Cette fonction doit retrouver une seule ligne principale dans le squelette nettoyé.

`extractMainPath` s'appuie sur `shortestPathLengths` pour mesurer les distances en nombre de pas dans le squelette, puis sur `reconstructPath` pour reconstruire le meilleur chemin.

Dans le cas le plus simple, la ligne principale est choisie comme la paire d'extrémités la plus éloignée en nombre de pas sur le squelette.

Une première visualisation intermédiaire est produite après cette étape :
- `tp5_main_paths.bmp`

Cette image permet de vérifier que la partie graphe du TP fonctionne déjà,
même si la partie géométrique n'est pas encore terminée.

### 4. Stabiliser la courbe et calculer la courbure globale
Complétez :
- `resamplePolyline`
- `smoothPolyline`
- `computeCurvatureMetrics`

`resamplePolyline` doit rééchantillonner la courbe à pas presque constant.
Le plus simple est de calculer une abscisse curviligne cumulative, puis d'interpoler de nouveaux points à des positions cibles régulières.

`smoothPolyline` permet de lisser légèrement la ligne extraite pour éviter une mesure trop instable.
Un lissage local simple suffit ici : on garde les extrémités fixes et on moyenne légèrement les points intérieurs.

`computeCurvatureMetrics` doit calculer au minimum :
- la longueur de corde ;
- la longueur d'arc ;
- la sagitta maximale ;
- le score global `sagitta / chordLength` ;
- un rayon et une courbure approchés.

Conseil : commencez par traiter proprement les cas dégénérés (`moins de deux points`, `corde quasi nulle`), puis calculez les mesures dans l'ordre demandé.

On adopte les définitions suivantes :
- `A` = premier point de la ligne principale ;
- `B` = dernier point de la ligne principale ;
- `AB` = la corde ;
- la **sagitta** = distance perpendiculaire maximale entre la ligne principale et la droite `(AB)`.

## Que se passe-t-il à partir du squelette ?

A partir du `skeleton`, le traitement se fait en plusieurs etapes.

1. **Lire le squelette comme un graphe**
Chaque pixel blanc du squelette est un noeud.
Deux pixels blancs voisins sont reliés.

2. **Identifier la structure locale**
Avec `skeletonNeighbors`, on compte les voisins de chaque pixel :
- degré 1 : extrémité
- degré 2 : point régulier
- degré >= 3 : jonction

3. **Suivre les branches simples**
`traceBranch` part d'une extrémité et avance jusqu'à :
- une jonction,
- une autre extrémité,
- ou une cassure.

Ça sert à isoler une branche locale.
Cette fonction sert surtout d'outil de lecture locale du squelette, en particulier pour le pruning.

4. **Nettoyer les petites branches parasites**
`pruneTerminalBranches` repere les extremites, suit leur branche avec `traceBranch`, puis supprime les petites branches terminales.

Cette étape nettoie le squelette, mais ne force pas encore une ligne principale unique.

5. **Explorer le squelette nettoyé**
`shortestPathLengths` lance un BFS depuis une extrémité pour calculer :
- la distance en nombre de pas vers tous les pixels
- le prédécesseur de chaque pixel

6. **Choisir la ligne principale**
`extractMainPath` compare les extrémités entre elles :
- on lance le BFS depuis une extrémité
- on cherche l'autre extrémité la plus lointaine
- on garde le meilleur couple

Puis on reconstruit ce chemin avec `reconstructPath`.

Contrairement à `traceBranch`, qui suit seulement une branche locale, `extractMainPath` prend ici une décision globale sur l'ensemble du squelette nettoyé.

7. **Transformer le chemin discret en courbe exploitable**
`resamplePolyline` rééchantillonne le chemin à pas presque constant.

8. **Lisser légèrement la courbe**
`smoothPolyline` réduit les petites irrégularités dues au squelette pixelisé.

9. **Calculer les mesures géométriques**
`computeCurvatureMetrics` calcule :
- la corde
- la longueur d'arc
- la sagitta
- le score de courbure
- un rayon / une courbure approchés

Donc, en une phrase :
à partir du squelette, on nettoie les petites branches, on choisit une ligne principale unique dans le graphe, puis on mesure sa courbure.

## Choix pédagogique
Dans ce TP, le squelette est traité comme un graphe non pondéré.

Autrement dit :
- chaque voisin actif compte pour un pas, y compris en diagonale ;
- la ligne principale est extraite par distance topologique sur le squelette ;
- les mesures géométriques réelles sont calculées ensuite sur la polyline extraite.

Ce choix est volontairement simple : il permet de rester centré sur la lecture du squelette comme graphe, sans introduire d'algorithme de plus court chemin pondéré.

Remarque : on ne cherche donc pas ici à approximer au mieux la longueur euclidienne sur le graphe ; on privilégie une méthode simple et robuste adaptée au niveau du TP.

## Fichiers produits
Le programme doit sauvegarder dans `out/` :
- `tp5_components.bmp` : visualisation des composantes connexes ;
- `tp5_skeleton.bmp` : squelette de toutes les composantes ;
- `tp5_main_paths.bmp` : résultat intermédiaire après extraction des lignes principales en pixels ;
- `tp5_paths.bmp` : lignes principales extraites avant l'affichage des cordes et points remarquables ;
- `tp5_viz.bmp` : visualisation complète avec les lignes principales et les cordes `AB`.

## Compilation
Depuis le dossier `tp5` :

Sous Windows :

```bash
g++ -std=c++17 -O2 -o tp5.exe tp5.cpp student_functions.cpp provided_functions.cpp ../io.cpp -lgdiplus
```

Sous Linux :

```bash
g++ -std=c++17 -O2 -o tp5 tp5.cpp student_functions.cpp provided_functions.cpp ../io.cpp
```

## Exécution
Avec les chemins par défaut :

```bash
.\tp5.exe
```

Avec un masque explicite et un seuil de pruning :

```bash
.\tp5.exe ..\data\GT\pointCloud_data_2026-02-19_12-03-32_rgb_semantic_mask.png 20
```

## Remarque sur les fichiers
Le programme principal est fourni dans `tp5.cpp`.

Le travail étudiant est concentré dans `student_functions.cpp`.

Les fonctions déjà codées et considérées comme acquises sont regroupées dans `provided_functions.cpp`.

## Vérifications attendues
- le programme doit charger une seule image GT ;
- `tp5_components.bmp` doit montrer les différentes composantes connexes ;
- `tp5_skeleton.bmp` doit montrer un squelette sur chaque composante connexe ;
- `tp5_main_paths.bmp` doit montrer les lignes principales extraites après l'étape graphe ;
- `tp5_paths.bmp` doit montrer au moins une ligne principale exploitable sur chaque composante retenue ;
- `tp5_viz.bmp` doit montrer une ligne principale et une corde `AB` sur chaque composante exploitable ;
- si le squelette est vide ou si la corde est quasi nulle, les mesures doivent devenir invalides proprement.

## Fichiers
- `tp5.cpp` : programme principal ;
- `student_config.h` : chemins et paramètres par défaut ;
- `provided_functions.h` : déclarations des fonctions fournies ;
- `provided_functions.cpp` : fonctions fournies ;
- `student_functions.h` : déclarations des fonctions à compléter ;
- `student_functions.cpp` : fonctions étudiantes ;
- `../io.cpp` : chargement et sauvegarde d'images.

## Duree
2h
