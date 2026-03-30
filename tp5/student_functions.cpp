#include "student_functions.h"
#include "provided_functions.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <queue>

namespace {

// Voisinage 8-connexe utilise partout pour parcourir le squelette.
constexpr int kNeighborCount = 8;
constexpr int kNeighborDx[kNeighborCount] = {-1, 0, 1, -1, 1, -1, 0, 1};
constexpr int kNeighborDy[kNeighborCount] = {-1, -1, -1, 0, 0, 1, 1, 1};

// Verifie que le pixel (x, y) reste dans l'image.
bool isInside(const Image8& image, int x, int y) {
    return x >= 0 && x < image.width && y >= 0 && y < image.height;
}

// Convertit des coordonnees 2D en indice lineaire dans le tableau de pixels.
size_t toIndex(const Image8& image, int x, int y) {
    return static_cast<size_t>(y) * image.width + x;
}

} // namespace

// Suit une branche simple du squelette a partir d'une extremite.
// Entree : le squelette binaire, un point de depart et son premier voisin.
// Sortie : la liste ordonnee des pixels de la branche jusqu'a une jonction ou une fin.
std::vector<PixelCoord> traceBranch(const Image8& skeleton, const PixelCoord& start, const PixelCoord& firstNeighbor) {
    // TODO :
    // 1. Initialiser la branche avec le point de depart et son premier voisin.
    // 2. Avancer pas a pas sur le squelette en regardant les voisins.
    // 3. Supprimer le pixel precedent pour savoir dans quelle direction continuer.
    // 4. Continuer tant que le point courant est un point regulier de degre 2.
    // 5. S'arreter quand on atteint une jonction, une extremite ou une cassure.


    std::vector<PixelCoord> path{start, firstNeighbor};
    PixelCoord previous = start;
    PixelCoord current = firstNeighbor;

    return path;
}

// Supprime iterativement les petites branches terminales parasites d'un squelette.
// Entree : un squelette binaire et une longueur maximale de branche a supprimer.
// Sortie : un squelette simplifie, reduit a sa structure principale.
Image8 pruneTerminalBranches(const Image8& skeleton, int minLength) {
    // TODO :
    // 1. Rechercher toutes les extremites du squelette.
    // 2. Pour chaque extremite, regarder les voisins, puis suivre la branche correspondante avec traceBranch.
    // 3. Si la branche est courte et qu'elle se termine sur une jonction, la supprimer.
    // 4. Recalculer les extremites sur le squelette mis a jour et iterer tant qu'une suppression modifie encore le resultat.

    Image8 result = skeleton;
    bool removedAny = true;


    // Apres le pruning local, il peut rester plusieurs morceaux de squelette.
    // On garde ici uniquement la plus grande composante
    return keepLargestComponent(result);
}

// Extrait une ligne principale unique a partir d'un squelette.
// Entree : un squelette binaire, en principe deja nettoye.
// Sortie : une suite ordonnee de pixels representant le chemin principal.
std::vector<PixelCoord> extractMainPath(const Image8& skeleton) {
    // TODO :
    // 1. Identifier les extremites du squelette.
    // 2. Si plusieurs extremites existent, chercher la paire d'extremites la plus eloignee
    //    en nombre de pas sur le squelette.
    //    Vous pouvez vous aider de shortestPathLengths pour calculer les distances depuis une extremite.
    //    Puis utiliser reconstructPath pour reconstruire le meilleur chemin trouve.
    // 3. Si le squelette est ambigu, utiliser une strategie de secours de type pseudo-diametre.
    // 4. Retourner la liste ordonnee des pixels de la ligne principale.

    std::vector<PixelCoord> endpoints;
    std::vector<PixelCoord> allPoints;

    // On recense tous les pixels du squelette et, parmi eux, les extremites.
    // TODO: Parcourir tous les pixels du squelette.
    // Pour chaque pixel actif, l'ajouter dans allPoints. Si c'est une extremite, l'ajouter aussi dans endpoints.


    // TODO : cas nominal : on teste chaque extremite comme point de depart, puis on garde
    // la paire d'extremites la plus eloignee en nombre de pas sur le squelette.
    
    if (endpoints.size() >= 2) {
        std::vector<PixelCoord> bestPath;
        double bestLength = -1.0;

        for (const PixelCoord& start : endpoints) {
            // TODO: calculer les distances et predecesseurs depuis start avec shortestPathLengths.
            
            // TODO: pour chaque extremite end differente de start, verifier si la distance est plus grande que bestLength.
        }

        return bestPath;
    }

    // Cas degrade : si on n'a pas deux extremites propres, on approxime le chemin principal
    // avec une recherche de pseudo-diametre sur le graphe.
    if (allPoints.size() < 2) {
        return allPoints;
    }

    // Strategie de secours : on approxime un long chemin du squelette par pseudo-diametre.
    // 1. On part d'un pixel quelconque du squelette.
    // 2. On cherche le pixel atteignable le plus lointain depuis ce point.
    const auto [firstDistances, _] = shortestPathLengths(skeleton, allPoints.front());
    size_t firstIndex = 0;
    for (size_t i = 0; i < firstDistances.size(); ++i) {
        if (std::isfinite(firstDistances[i]) && firstDistances[i] > firstDistances[firstIndex]) {
            firstIndex = i;
        }
    }

    // On convertit l'indice lineaire du pixel le plus lointain en coordonnees (x, y).
    PixelCoord farthest{static_cast<int>(firstIndex % skeleton.width), static_cast<int>(firstIndex / skeleton.width)};

    // 3. On relance un BFS depuis ce pixel lointain.
    // 4. Le pixel le plus lointain obtenu cette fois sert d'autre extremite.
    const auto [distances, previous] = shortestPathLengths(skeleton, farthest);
    size_t secondIndex = firstIndex;
    for (size_t i = 0; i < distances.size(); ++i) {
        if (std::isfinite(distances[i]) && distances[i] > distances[secondIndex]) {
            secondIndex = i;
        }
    }

    PixelCoord other{static_cast<int>(secondIndex % skeleton.width), static_cast<int>(secondIndex / skeleton.width)};
    return reconstructPath(skeleton, previous, other);
}

// Reechantillonne une polyline pour obtenir des points presque regulierement espaces.
// Entree : un chemin discret de pixels et un pas cible.
// Sortie : une polyline reelle plus stable pour les mesures geometriques.
std::vector<Point2d> resamplePolyline(const std::vector<PixelCoord>& path, double spacing) {
    // TODO :
    // 1. Convertir le chemin discret en points reels.
    // 2. Calculer l'abscisse curviligne cumulative le long de la polyline.
    //    Autrement dit : pour chaque point, memoriser la longueur deja parcourue.
    // 3. Reechantillonner la courbe a intervalles presque constants.
    //    On placera ensuite de nouveaux points par interpolation lineaire.

    // On convertit d'abord le chemin discret en points reels.
    std::vector<Point2d> points;
    // TODO: Convertir chaque PixelCoord en Point2d en utilisant des conversions explicites de type static_cast<double>.


    if (points.size() < 2) {
        return points;
    }

    // cumulative[i] indique la longueur deja parcourue le long de la ligne pour arriver au point i.
    std::vector<double> cumulative(points.size(), 0.0);
    // TODO: A chaque etape, on ajoute la longueur du segment entre deux points consecutifs.
    // std::hypot(dx, dy) calcule la longueur du petit segment entre deux points consecutifs.


    const double total = cumulative.back();
    if (total <= 1e-12) {
        return {points.front()};
    }

    // Puis on replace des echantillons a intervalles presque constants.
    // Si la courbe a une longueur totale L et que l'on vise un pas 'spacing',
    // on veut environ un point tous les 'spacing' unites, en conservant aussi
    // le premier et le dernier point de la polyline.
    const int sampleCount = std::max(2, static_cast<int>(std::ceil(total / spacing)) + 1);
    std::vector<Point2d> samples;


    for (int s = 0; s < sampleCount; ++s) {
        // Pour chaque nouveau point, on fixe une position cible le long de la courbe,
        // exprimee en abscisse curviligne entre 0 et la longueur totale.
        const double target = total * s / (sampleCount - 1);
        size_t idx = 1;
        // Chercher entre quels deux points de la ligne originale cette position se trouve.
        while (idx < cumulative.size() && cumulative[idx] < target) {
            ++idx;
        }
        idx = std::min(idx, cumulative.size() - 1);
        // Mesurer la longueur de ce segment local.
        const double segLength = cumulative[idx] - cumulative[idx - 1];
        if (segLength <= 1e-12) {
            samples.push_back(points[idx]);
            continue;
        }
        const double alpha = (target - cumulative[idx - 1]) / segLength;
        samples.push_back({
            (1.0 - alpha) * points[idx - 1].x + alpha * points[idx].x,
            (1.0 - alpha) * points[idx - 1].y + alpha * points[idx].y
        });
    }

    return samples;
}

// Applique un lissage local simple a une polyline.
// Entree : une suite de points et un nombre d'iterations.
// Sortie : une courbe legerement regularisee.
std::vector<Point2d> smoothPolyline(const std::vector<Point2d>& points, int iterations) {
    // TODO : Lisser legerement la ligne principale pour stabiliser la mesure de courbure.


    std::vector<Point2d> result = points;
    if (result.size() < 3) {
        return result;
    }

    // TODO: Lissage local simple : chaque point interieur devient une moyenne ponderee de ses voisins.
    // Garder les deux extremites fixes permet de conserver la corde AB finale.


    return result;
}

// Calcule les mesures geometriques globales d'une ligne principale.
// Entree : une polyline ordonnee representant la courbe centrale de l'objet.
// Sortie : la corde, la longueur d'arc, la sagitta et des derivees de courbure.
CurvatureMetrics computeCurvatureMetrics(const std::vector<Point2d>& points) {
    // TODO :
    // 1. Definir A et B comme les deux extremites de la ligne principale.
    // 2. Calculer la longueur de la corde AB.
    // 3. Calculer la longueur d'arc de la polyline.
    // 4. Chercher le point le plus eloigne de la droite (AB) pour obtenir la sagitta.
    // 5. En deduire le score de courbure et un rayon de cercle approche.
    // Commencer par les cas degeneres permet d'eviter des divisions par zero plus loin.

    CurvatureMetrics metrics;

    // TODO:  Il faut au moins deux points pour definir la corde AB.


    // TODO: A et B sont les deux extremites de la ligne principale.
    // Stocker les extremites dans metrics.start et metrics.end et metrics.chordLength pour les utiliser plus tard.


    if (metrics.chordLength <= 1e-12) {
        metrics.reason = "zero_chord";
        return metrics;
    }

    // TODO: Longueur d'arc de la ligne principale (Distance totale parcourue le long de la ligne principale).

    // La sagitta est la distance maximale entre la courbe et la droite (AB).
    // On cherche donc le point de la polyline le plus eloigne de cette droite :
    // ce point sera memorise dans metrics.maxPoint.
    double bestDistance = -1.0;
    for (const Point2d& point : points) {
        const double distance = pointLineDistance(point, metrics.start, metrics.end);
        if (distance > bestDistance) {
            bestDistance = distance;
            metrics.maxPoint = point;
        }
    }

    metrics.sagitta = std::max(0.0, bestDistance);
    metrics.curvatureScore = metrics.sagitta / metrics.chordLength;

    // On calcule aussi un rayon de cercle approche a partir de A, B et du point de sagitta max.
    metrics.circleRadius = circleRadiusFromThreePoints(metrics.start, metrics.maxPoint, metrics.end);
    metrics.circleCurvature = (metrics.circleRadius > 1e-12) ? 1.0 / metrics.circleRadius : 0.0;
    metrics.valid = true;
    return metrics;
}

std::pair<std::vector<double>, std::vector<int>> shortestPathLengths(const Image8& skeleton, const PixelCoord& start);

// Cette fonction explore le squelette a partir d'un point de depart avec un BFS.
// Ici, on voit le squelette comme un graphe non pondere : chaque voisin actif
// compte pour un pas, y compris en diagonale.
//
// Idee a retenir :
// - on part du pixel start ;
// - on explore ensuite le squelette couche par couche ;
// - le premier passage sur un pixel donne sa distance minimale en nombre de pas ;
// - on memorise aussi d'ou l'on vient pour pouvoir reconstruire un chemin plus tard.
//
// Entree : un squelette binaire et un point de depart.
// Sortie :
// - distances[i] = distance en nombre de pas depuis le point de depart jusqu'au pixel i ;
// - previous[i] = pixel precedent sur le chemin permettant d'atteindre i.
std::pair<std::vector<double>, std::vector<int>> shortestPathLengths(const Image8& skeleton, const PixelCoord& start) {
    // creer un tableau distances rempli avec +infini.
    // Cela signifie qu'au debut, aucun pixel n'a encore ete atteint.
    std::vector<double> distances(skeleton.data.size(), std::numeric_limits<double>::infinity());
    // creer un tableau previous rempli avec -1.
    // La valeur -1 signifie ici : pas encore de predecesseur connu.
    std::vector<int> previous(skeleton.data.size(), -1);
    // file frontier pour explorer le squelette en largeur.
    std::queue<PixelCoord> frontier;

    // le point de depart est connu a distance 0.
    distances[toIndex(skeleton, start.x, start.y)] = 0.0;
    // Ajouter le start dans la file pour lancer l'exploration.
    frontier.push(start);

    // Invariant BFS : quand un pixel sort de la file, sa distance minimale
    // en nombre de pas est deja fixee.
    // TODO 0 : tant que la file n'est pas vide, continuer a explorer le squelette.
    // TODO 1 : recuperer le pixel en tete de file, puis le retirer. Utiliser son index avec la fonction toIndex.
    // TODO 2 : parcourir tous les voisins actifs du pixel courant avec la fonction skeletonNeighbors.
    // TODO 3 : si ce voisin a deja une distance finie, il a deja ete decouvert.
    // Dans ce cas, on ne le traite pas une deuxieme fois.
    // TODO 4 : un voisin decouvert pour la premiere fois est a un pas de plus.
    // TODO 5 : memoriser que l'on a atteint ce voisin depuis current.
    // TODO 6 : ajouter ce voisin dans la file pour poursuivre le BFS.


    return {distances, previous};
}
