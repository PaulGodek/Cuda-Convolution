#include "provided_functions.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <queue>

namespace {

// Voisinage 8-connexe utilise dans les parcours d'image et de squelette.
constexpr int kNeighborCount = 8;
constexpr int kNeighborDx[kNeighborCount] = {-1, 0, 1, -1, 1, -1, 0, 1};
constexpr int kNeighborDy[kNeighborCount] = {-1, -1, -1, 0, 0, 1, 1, 1};

// Niveaux de gris cycliques pour distinguer visuellement les composantes connexes.
constexpr uint8_t kComponentLevels[] = {48, 80, 112, 144, 176, 208, 240};

// Verifie que le pixel (x, y) reste dans l'image.
bool isInside(const Image8& image, int x, int y) {
    return x >= 0 && x < image.width && y >= 0 && y < image.height;
}

// Convertit des coordonnees 2D en indice lineaire dans le tableau 1D.
size_t toIndex(const Image8& image, int x, int y) {
    return static_cast<size_t>(y) * image.width + x;
}

// Ecrit un pixel si la coordonnee est valide.
void setPixel(Image8& image, int x, int y, uint8_t value) {
    if (isInside(image, x, y)) {
        image.data[toIndex(image, x, y)] = value;
    }
}

// Trace un segment sur l'image de visualisation.
void drawLine(Image8& image, Point2d a, Point2d b, uint8_t value) {
    int x0 = static_cast<int>(std::lround(a.x));
    int y0 = static_cast<int>(std::lround(a.y));
    const int x1 = static_cast<int>(std::lround(b.x));
    const int y1 = static_cast<int>(std::lround(b.y));
    const int dx = std::abs(x1 - x0);
    const int sx = (x0 < x1) ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        setPixel(image, x0, y0, value);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        const int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Dessine une petite croix sur un point remarquable.
void drawCross(Image8& image, Point2d center, int radius, uint8_t value) {
    const int cx = static_cast<int>(std::lround(center.x));
    const int cy = static_cast<int>(std::lround(center.y));
    for (int dx = -radius; dx <= radius; ++dx) {
        setPixel(image, cx + dx, cy, value);
    }
    for (int dy = -radius; dy <= radius; ++dy) {
        setPixel(image, cx, cy + dy, value);
    }
}

} // namespace

Image8 thresholdMask(const Image8& input) {
    Image8 result;
    result.width = input.width;
    result.height = input.height;
    result.data.resize(input.data.size(), 0);

    // Toute valeur non nulle devient un pixel blanc du masque binaire.
    for (size_t i = 0; i < input.data.size(); ++i) {
        result.data[i] = (input.data[i] != 0) ? 255 : 0;
    }

    return result;
}

std::vector<std::vector<PixelCoord>> extractConnectedComponents(const Image8& mask, int minComponentSize) {
    std::vector<std::vector<PixelCoord>> components;
    if (mask.width <= 0 || mask.height <= 0 || mask.data.empty()) {
        return components;
    }

    // BFS classique sur image binaire pour recuperer toutes les composantes 8-connexes.
    std::vector<uint8_t> visited(mask.data.size(), 0);
    std::queue<PixelCoord> frontier;

    for (int y = 0; y < mask.height; ++y) {
        for (int x = 0; x < mask.width; ++x) {
            const size_t startIndex = toIndex(mask, x, y);
            if (mask.data[startIndex] == 0 || visited[startIndex] != 0) {
                continue;
            }

            std::vector<PixelCoord> component;
            frontier.push({x, y});
            visited[startIndex] = 1;

            while (!frontier.empty()) {
                const PixelCoord current = frontier.front();
                frontier.pop();
                component.push_back(current);

                for (int k = 0; k < kNeighborCount; ++k) {
                    const int nx = current.x + kNeighborDx[k];
                    const int ny = current.y + kNeighborDy[k];
                    if (!isInside(mask, nx, ny)) {
                        continue;
                    }
                    const size_t neighborIndex = toIndex(mask, nx, ny);
                    if (mask.data[neighborIndex] == 0 || visited[neighborIndex] != 0) {
                        continue;
                    }
                    visited[neighborIndex] = 1;
                    frontier.push({nx, ny});
                }
            }

            if (static_cast<int>(component.size()) >= minComponentSize) {
                components.push_back(component);
            }
        }
    }

    return components;
}

Image8 buildComponentPreview(const Image8& binaryMask, const std::vector<std::vector<PixelCoord>>& components) {
    Image8 preview;
    preview.width = binaryMask.width;
    preview.height = binaryMask.height;
    preview.data.assign(binaryMask.data.size(), 0);

    // Chaque composante recoit un niveau de gris pour la visualisation.
    for (size_t i = 0; i < components.size(); ++i) {
        const uint8_t value = kComponentLevels[i % (sizeof(kComponentLevels) / sizeof(kComponentLevels[0]))];
        for (const PixelCoord& pixel : components[i]) {
            preview.data[toIndex(preview, pixel.x, pixel.y)] = value;
        }
    }

    return preview;
}

Image8 buildMaskFromComponent(const Image8& referenceMask, const std::vector<PixelCoord>& component) {
    Image8 result;
    result.width = referenceMask.width;
    result.height = referenceMask.height;
    result.data.assign(referenceMask.data.size(), 0);

    // On reconstruit un masque pleine image ne contenant qu'une seule composante.
    for (const PixelCoord& pixel : component) {
        result.data[toIndex(result, pixel.x, pixel.y)] = 255;
    }

    return result;
}

CropBox computeBoundingBox(const Image8& mask, int padding) {
    CropBox crop{mask.width, mask.height, 0, 0};
    bool found = false;

    // Recherche de la plus petite boite englobante couvrant tous les pixels actifs.
    for (int y = 0; y < mask.height; ++y) {
        for (int x = 0; x < mask.width; ++x) {
            if (mask.data[toIndex(mask, x, y)] == 0) {
                continue;
            }
            found = true;
            crop.minX = std::min(crop.minX, x);
            crop.minY = std::min(crop.minY, y);
            crop.maxX = std::max(crop.maxX, x + 1);
            crop.maxY = std::max(crop.maxY, y + 1);
        }
    }

    if (!found) {
        return {0, 0, 0, 0};
    }

    // On ajoute une petite marge pour ne pas coller le traitement au bord de la ROI.
    crop.minX = std::max(0, crop.minX - padding);
    crop.minY = std::max(0, crop.minY - padding);
    crop.maxX = std::min(mask.width, crop.maxX + padding);
    crop.maxY = std::min(mask.height, crop.maxY + padding);
    return crop;
}

Image8 cropMask(const Image8& mask, const CropBox& cropBox) {
    Image8 result;
    result.width = std::max(0, cropBox.maxX - cropBox.minX);
    result.height = std::max(0, cropBox.maxY - cropBox.minY);
    result.data.assign(static_cast<size_t>(result.width) * result.height, 0);

    // Copie de la region d'interet dans une image locale plus petite.
    for (int y = 0; y < result.height; ++y) {
        for (int x = 0; x < result.width; ++x) {
            result.data[toIndex(result, x, y)] = mask.data[toIndex(mask, cropBox.minX + x, cropBox.minY + y)];
        }
    }

    return result;
}

void pasteMask(Image8& destination, const Image8& source, const CropBox& cropBox) {
    // Replace une ROI locale dans l'image globale.
    for (int y = 0; y < source.height; ++y) {
        for (int x = 0; x < source.width; ++x) {
            const uint8_t value = source.data[toIndex(source, x, y)];
            if (value != 0) {
                destination.data[toIndex(destination, cropBox.minX + x, cropBox.minY + y)] = value;
            }
        }
    }
}

Image8 zhangSuenThinning(const Image8& mask) {
    Image8 image = thresholdMask(mask);
    bool changed = true;

    // Skeletonization iterative de Zhang-Suen.
    // Cette partie est fournie pour que le TP reste centre sur le graphe du squelette.
    while (changed) {
        changed = false;
        for (int step = 0; step < 2; ++step) {
            std::vector<uint8_t> toDelete(image.data.size(), 0);
            for (int y = 1; y < image.height - 1; ++y) {
                for (int x = 1; x < image.width - 1; ++x) {
                    const size_t index = toIndex(image, x, y);
                    if (image.data[index] == 0) {
                        continue;
                    }

                    const uint8_t p2 = image.data[toIndex(image, x, y - 1)] ? 1 : 0;
                    const uint8_t p3 = image.data[toIndex(image, x + 1, y - 1)] ? 1 : 0;
                    const uint8_t p4 = image.data[toIndex(image, x + 1, y)] ? 1 : 0;
                    const uint8_t p5 = image.data[toIndex(image, x + 1, y + 1)] ? 1 : 0;
                    const uint8_t p6 = image.data[toIndex(image, x, y + 1)] ? 1 : 0;
                    const uint8_t p7 = image.data[toIndex(image, x - 1, y + 1)] ? 1 : 0;
                    const uint8_t p8 = image.data[toIndex(image, x - 1, y)] ? 1 : 0;
                    const uint8_t p9 = image.data[toIndex(image, x - 1, y - 1)] ? 1 : 0;

                    const int neighbors = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
                    const int transitions =
                        (p2 == 0 && p3 == 1) +
                        (p3 == 0 && p4 == 1) +
                        (p4 == 0 && p5 == 1) +
                        (p5 == 0 && p6 == 1) +
                        (p6 == 0 && p7 == 1) +
                        (p7 == 0 && p8 == 1) +
                        (p8 == 0 && p9 == 1) +
                        (p9 == 0 && p2 == 1);

                    if (neighbors < 2 || neighbors > 6 || transitions != 1) {
                        continue;
                    }

                    if (step == 0) {
                        if (p2 * p4 * p6 != 0 || p4 * p6 * p8 != 0) {
                            continue;
                        }
                    } else {
                        if (p2 * p4 * p8 != 0 || p2 * p6 * p8 != 0) {
                            continue;
                        }
                    }

                    toDelete[index] = 1;
                }
            }

            for (size_t i = 0; i < image.data.size(); ++i) {
                if (toDelete[i] != 0) {
                    image.data[i] = 0;
                    changed = true;
                }
            }
        }
    }

    return image;
}

std::vector<PixelCoord> skeletonNeighbors(const Image8& skeleton, const PixelCoord& point) {
    // Fonction fournie : retourne simplement les voisins actifs 8-connexes d'un pixel.
    std::vector<PixelCoord> neighbors;

    for (int k = 0; k < kNeighborCount; ++k) {
        const int nx = point.x + kNeighborDx[k];
        const int ny = point.y + kNeighborDy[k];
        if (isInside(skeleton, nx, ny) && skeleton.data[toIndex(skeleton, nx, ny)] != 0) {
            neighbors.push_back({nx, ny});
        }
    }

    return neighbors;
}

Image8 keepLargestComponent(const Image8& mask) {
    // Apres suppression de branches, on garde la plus grande composante restante.
    const std::vector<std::vector<PixelCoord>> components = extractConnectedComponents(mask, 1);
    Image8 result;
    result.width = mask.width;
    result.height = mask.height;
    result.data.assign(mask.data.size(), 0);

    if (components.empty()) {
        return result;
    }

    const auto& largest = *std::max_element(
        components.begin(),
        components.end(),
        [](const auto& a, const auto& b) { return a.size() < b.size(); }
    );

    for (const PixelCoord& pixel : largest) {
        result.data[toIndex(result, pixel.x, pixel.y)] = 255;
    }

    return result;
}

// Reconstruit un chemin a partir du tableau des predecesseurs.
//
// Entree :
// - skeleton : image servant a convertir un indice lineaire en coordonnees (x, y) ;
// - previous : tableau des predecesseurs calcule pendant le parcours ;
// - end : point final du chemin a reconstruire.
//
// Sortie :
// - la liste ordonnee des pixels du chemin, du debut jusqu'au point final.
std::vector<PixelCoord> reconstructPath(const Image8& skeleton, const std::vector<int>& previous, const PixelCoord& end) {
    // On remonte le tableau des predecesseurs depuis la fin vers le debut.
    std::vector<PixelCoord> path;
    int current = static_cast<int>(toIndex(skeleton, end.x, end.y));
    while (current >= 0) {
        path.push_back({current % skeleton.width, current / skeleton.width});
        current = previous[static_cast<size_t>(current)];
    }

    std::reverse(path.begin(), path.end());
    return path;
}

// Calcule la distance entre un point et la droite definie par deux autres points.
//
// Entree :
// - point : point dont on veut mesurer l'ecart ;
// - start et end : deux points qui definissent la droite.
//
// Sortie :
// - la distance perpendiculaire entre point et la droite (start, end).
double pointLineDistance(Point2d point, Point2d start, Point2d end) {
    // Distance point-droite classique en 2D.
    const double dx = end.x - start.x;
    const double dy = end.y - start.y;
    const double norm = std::sqrt(dx * dx + dy * dy);
    if (norm <= 1e-12) {
        return 0.0;
    }
    return std::abs(dx * (point.y - start.y) - dy * (point.x - start.x)) / norm;
}

// Calcule le rayon du cercle passant par trois points.
//
// Entree :
// - a, b, c : trois points du plan.
//
// Sortie :
// - le rayon du cercle passant par ces trois points ;
// - si les trois points sont presque alignes, la fonction renvoie 0.
double circleRadiusFromThreePoints(Point2d a, Point2d b, Point2d c) {
    // Formule geometrique du rayon via les trois cotes et l'aire du triangle.
    const double sideAB = std::hypot(a.x - b.x, a.y - b.y);
    const double sideBC = std::hypot(b.x - c.x, b.y - c.y);
    const double sideCA = std::hypot(c.x - a.x, c.y - a.y);
    const double twiceArea = std::abs((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x));
    if (twiceArea <= 1e-12) {
        return 0.0;
    }
    return (sideAB * sideBC * sideCA) / (2.0 * twiceArea);
}

Image8 drawPixelPathPreview(const Image8& binaryMask, const Image8& skeletonAll, const std::vector<std::vector<PixelCoord>>& paths, const std::vector<CropBox>& cropBoxes) {
    Image8 preview;
    preview.width = binaryMask.width;
    preview.height = binaryMask.height;
    preview.data.assign(binaryMask.data.size(), 0);

    // Fond sombre pour l'objet, gris moyen pour le squelette global.
    for (size_t i = 0; i < binaryMask.data.size(); ++i) {
        preview.data[i] = (binaryMask.data[i] != 0) ? 40 : 0;
        if (skeletonAll.data[i] != 0) {
            preview.data[i] = 120;
        }
    }

    const size_t count = std::min(paths.size(), cropBoxes.size());
    for (size_t i = 0; i < count; ++i) {
        const std::vector<PixelCoord>& path = paths[i];
        const CropBox& cropBox = cropBoxes[i];
        for (size_t j = 1; j < path.size(); ++j) {
            const Point2d a{static_cast<double>(path[j - 1].x + cropBox.minX), static_cast<double>(path[j - 1].y + cropBox.minY)};
            const Point2d b{static_cast<double>(path[j].x + cropBox.minX), static_cast<double>(path[j].y + cropBox.minY)};
            drawLine(preview, a, b, 220);
        }
    }

    return preview;
}

Image8 drawPathPreview(const Image8& binaryMask, const Image8& skeletonAll, const std::vector<std::vector<Point2d>>& paths) {
    Image8 preview;
    preview.width = binaryMask.width;
    preview.height = binaryMask.height;
    preview.data.assign(binaryMask.data.size(), 0);

    // Fond sombre pour l'objet, gris moyen pour le squelette global.
    for (size_t i = 0; i < binaryMask.data.size(); ++i) {
        preview.data[i] = (binaryMask.data[i] != 0) ? 40 : 0;
        if (skeletonAll.data[i] != 0) {
            preview.data[i] = 120;
        }
    }

    // On trace seulement les lignes principales extraites, sans les metriques finales.
    for (const std::vector<Point2d>& path : paths) {
        for (size_t j = 1; j < path.size(); ++j) {
            drawLine(preview, path[j - 1], path[j], 220);
        }
    }

    return preview;
}

Image8 drawVisualization(const Image8& binaryMask, const Image8& skeletonAll, const std::vector<std::vector<Point2d>>& paths, const std::vector<CurvatureMetrics>& metricsList) {
    Image8 viz;
    viz.width = binaryMask.width;
    viz.height = binaryMask.height;
    viz.data.assign(binaryMask.data.size(), 0);

    // Fond sombre pour l'objet, gris moyen pour le squelette global.
    for (size_t i = 0; i < binaryMask.data.size(); ++i) {
        viz.data[i] = (binaryMask.data[i] != 0) ? 40 : 0;
        if (skeletonAll.data[i] != 0) {
            viz.data[i] = 120;
        }
    }

    // Pour chaque composante valide :
    // - on trace la ligne principale ;
    // - on trace la corde AB ;
    // - on marque A, B et le point de sagitta maximale.
    const size_t count = std::min(paths.size(), metricsList.size());
    for (size_t i = 0; i < count; ++i) {
        const std::vector<Point2d>& path = paths[i];
        const CurvatureMetrics& metrics = metricsList[i];

        for (size_t j = 1; j < path.size(); ++j) {
            drawLine(viz, path[j - 1], path[j], 200);
        }

        if (metrics.valid) {
            drawLine(viz, metrics.start, metrics.end, 255);
            drawCross(viz, metrics.start, 3, 220);
            drawCross(viz, metrics.end, 3, 220);
            drawCross(viz, metrics.maxPoint, 3, 180);
        }
    }

    return viz;
}
