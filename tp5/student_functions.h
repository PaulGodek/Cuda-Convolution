#ifndef STUDENT_FUNCTIONS_H
#define STUDENT_FUNCTIONS_H

#include "../io.h"
#include <string>
#include <utility>
#include <vector>

// Coordonnees entieres d'un pixel dans l'image.
struct PixelCoord {
    int x = 0;
    int y = 0;
};

// Coordonnees reelles d'un point le long de la courbe principale.
struct Point2d {
    double x = 0.0;
    double y = 0.0;
};

// Boite de recadrage d'une composante dans l'image originale.
struct CropBox {
    int minX = 0;
    int minY = 0;
    int maxX = 0;
    int maxY = 0;
};

// Mesures geometriques calculees sur une ligne principale.
struct CurvatureMetrics {
    bool valid = false;
    std::string reason;
    double chordLength = 0.0;
    double arcLength = 0.0;
    double sagitta = 0.0;
    double curvatureScore = 0.0;
    double circleRadius = 0.0;
    double circleCurvature = 0.0;
    Point2d start;
    Point2d end;
    Point2d maxPoint;
};

// ==============================
// Fonctions a completer par l'etudiant
// ==============================

// Suit une branche depuis une extremite jusqu'a une jonction ou une fin.
std::vector<PixelCoord> traceBranch(const Image8& skeleton, const PixelCoord& start, const PixelCoord& firstNeighbor);

// Supprime les petites branches terminales parasites.
Image8 pruneTerminalBranches(const Image8& skeleton, int minLength);

// Calcule les distances en nombre de pas et les predecesseurs depuis un point de depart.
std::pair<std::vector<double>, std::vector<int>> shortestPathLengths(const Image8& skeleton, const PixelCoord& start);

// Extrait une ligne principale unique dans le squelette.
std::vector<PixelCoord> extractMainPath(const Image8& skeleton);

// Reechantillonne la ligne principale a pas presque constant.
std::vector<Point2d> resamplePolyline(const std::vector<PixelCoord>& path, double spacing);

// Lissage simple de la polyline finale.
std::vector<Point2d> smoothPolyline(const std::vector<Point2d>& points, int iterations);

// Calcule la corde, la sagitta et les mesures derivees.
CurvatureMetrics computeCurvatureMetrics(const std::vector<Point2d>& points);

#endif // STUDENT_FUNCTIONS_H
