#include "../io.h"
#include "student_config.h"
#include "student_functions.h"
#include "provided_functions.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

namespace {

// Lit un entier positif depuis la ligne de commande.
// Si la conversion echoue, on garde la valeur par defaut.
int parseOrDefault(const char* value, int fallback) {
    if (value == nullptr) {
        return fallback;
    }
    const int parsed = std::atoi(value);
    return (parsed > 0) ? parsed : fallback;
}

// Verifie rapidement si une image binaire contient au moins un pixel actif.
bool hasForeground(const Image8& image) {
    for (uint8_t value : image.data) {
        if (value != 0) {
            return true;
        }
    }
    return false;
}

// Normalise l'affichage d'un chemin dans la console.
std::string normalizePath(const std::filesystem::path& path) {
    return path.lexically_normal().string();
}

} // namespace

int main(int argc, char** argv) {
    const auto startTime = std::chrono::steady_clock::now();

    // Parametres : chemin du masque et seuil de pruning optionnel.
    const std::string groundTruthPath = (argc >= 2) ? argv[1] : kStudentConfig.defaultGroundTruthPath;
    const int branchLengthThreshold = (argc >= 3) ? parseOrDefault(argv[2], kStudentConfig.defaultBranchLengthThreshold) : kStudentConfig.defaultBranchLengthThreshold;

    // 1. Chargement et binarisation du masque d'entree.
    const Image8 input = loadImage(groundTruthPath);
    const Image8 binaryMask = thresholdMask(input);

    // 2. Extraction des composantes connexes sur l'image complete.
    const std::vector<std::vector<PixelCoord>> components = extractConnectedComponents(binaryMask, kStudentConfig.defaultMinComponentSize);

    // 3. Preparation du dossier de sortie et des noms de fichiers generes.
    const std::filesystem::path outDir = std::filesystem::current_path() / "out";
    std::filesystem::create_directories(outDir);
    const std::string componentsPath = normalizePath(outDir / "tp5_components.bmp");
    const std::string skeletonPath = normalizePath(outDir / "tp5_skeleton.bmp");
    const std::string mainPathsPath = normalizePath(outDir / "tp5_main_paths.bmp");
    const std::string pathsPath = normalizePath(outDir / "tp5_paths.bmp");
    const std::string vizPath = normalizePath(outDir / "tp5_viz.bmp");

    // Sauvegarde d'une image de controle avec les composantes connexes colorees en niveaux de gris.
    saveImage(buildComponentPreview(binaryMask, components), componentsPath);

    // Image globale qui recevra le squelette de toutes les composantes.
    Image8 skeletonAll;
    skeletonAll.width = binaryMask.width;
    skeletonAll.height = binaryMask.height;
    skeletonAll.data.assign(binaryMask.data.size(), 0);

    // Ces tableaux servent au rendu final : une courbe principale et des metriques par composante valide.
    std::vector<std::vector<PixelCoord>> allMainPathPixels;
    std::vector<CropBox> allMainPathCropBoxes;
    std::vector<std::vector<Point2d>> allPaths;
    std::vector<CurvatureMetrics> allMetrics;

    // On retient aussi une composante de reference pour afficher un resume numerique dans la console.
    int referenceComponentIndex = -1;
    int referenceComponentSize = 0;
    CurvatureMetrics referenceMetrics;

    // 4. Traitement de chaque composante connexe separement.
    for (size_t i = 0; i < components.size(); ++i) {
        // On reconstruit un masque pleine image qui ne contient que la composante courante.
        const Image8 componentMask = buildMaskFromComponent(binaryMask, components[i]);

        // On recadre sur la boite englobante pour accelerer la skeletonization.
        const CropBox cropBox = computeBoundingBox(componentMask, kStudentConfig.defaultCropPadding);
        const Image8 croppedMask = cropMask(componentMask, cropBox);
        if (croppedMask.width == 0 || croppedMask.height == 0) {
            continue;
        }

        // 5. Calcul du squelette local sur la ROI de la composante.
        Image8 skeletonROI = zhangSuenThinning(croppedMask);
        skeletonROI = thresholdMask(skeletonROI);
        if (!hasForeground(skeletonROI)) {
            continue;
        }

        // On remet ce squelette local dans l'image globale.
        pasteMask(skeletonAll, skeletonROI, cropBox);

        // Petit pipeline local :
        // squelette -> chemin principal -> reechantillonnage -> lissage -> metriques.
        auto extractPathPixels = [&](const Image8& localSkeleton) {
            return extractMainPath(localSkeleton);
        };

        auto processPathGeometry = [&](const std::vector<PixelCoord>& pathPixels) {
            std::vector<Point2d> path = resamplePolyline(pathPixels, kStudentConfig.defaultResampleSpacing);
            path = smoothPolyline(path, kStudentConfig.defaultSmoothingIterations);

            // Les points ont ete calcules dans la ROI locale : on les remonte dans l'image complete.
            for (Point2d& point : path) {
                point.x += cropBox.minX;
                point.y += cropBox.minY;
            }
            return std::make_pair(path, computeCurvatureMetrics(path));
        };

        // On tente d'abord la version nettoyee du squelette.
        Image8 prunedSkeleton = pruneTerminalBranches(skeletonROI, branchLengthThreshold);
        std::vector<PixelCoord> pathPixels = extractPathPixels(prunedSkeleton);
        bool usedRawSkeletonForPreview = false;

        // Si le pruning a trop simplifie ou casse le chemin principal, on retombe sur le squelette brut.
        if (pathPixels.empty()) {
            pathPixels = extractPathPixels(skeletonROI);
            usedRawSkeletonForPreview = true;
        }

        // Premiere sortie intermediaire du TP : la ligne principale extraite apres l'etape graphe.
        if (!pathPixels.empty()) {
            allMainPathPixels.push_back(pathPixels);
            allMainPathCropBoxes.push_back(cropBox);
        }

        auto [path, metrics] = processPathGeometry(pathPixels);

        // Si la geometrie devient invalide, on retombe sur le squelette brut.
        if (!metrics.valid) {
            const std::vector<PixelCoord> fallbackPathPixels = extractPathPixels(skeletonROI);
            if (!usedRawSkeletonForPreview && !fallbackPathPixels.empty()) {
                allMainPathPixels.push_back(fallbackPathPixels);
                allMainPathCropBoxes.push_back(cropBox);
            }
            std::tie(path, metrics) = processPathGeometry(fallbackPathPixels);
        }

        if (!metrics.valid) {
            continue;
        }

        // Cette composante est exploitable : on la garde pour l'image finale.
        allPaths.push_back(path);
        allMetrics.push_back(metrics);

        // La composante de reference est simplement la plus grande composante valide.
        if (static_cast<int>(components[i].size()) > referenceComponentSize) {
            referenceComponentSize = static_cast<int>(components[i].size());
            referenceComponentIndex = static_cast<int>(i);
            referenceMetrics = metrics;
        }
    }

    // 6. Sauvegarde des sorties image.
    saveImage(skeletonAll, skeletonPath);
    saveImage(drawPixelPathPreview(binaryMask, skeletonAll, allMainPathPixels, allMainPathCropBoxes), mainPathsPath);
    saveImage(drawPathPreview(binaryMask, skeletonAll, allPaths), pathsPath);
    saveImage(drawVisualization(binaryMask, skeletonAll, allPaths, allMetrics), vizPath);

    const auto endTime = std::chrono::steady_clock::now();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // 7. Resume console des mesures obtenues.
    std::cout << "Ground truth: " << groundTruthPath << '\n';
    std::cout << "Branch length threshold: " << branchLengthThreshold << '\n';
    std::cout << "Mask size: " << binaryMask.width << " x " << binaryMask.height << '\n';
    std::cout << "Connected components: " << components.size() << '\n';
    std::cout << "Valid curved components: " << allMetrics.size() << '\n';
    std::cout << "Reference component index: " << referenceComponentIndex << '\n';
    std::cout << "Reference component size: " << referenceComponentSize << '\n';
    std::cout << "Metrics valid: " << referenceMetrics.valid << '\n';
    std::cout << "Chord length: " << referenceMetrics.chordLength << '\n';
    std::cout << "Arc length: " << referenceMetrics.arcLength << '\n';
    std::cout << "Sagitta: " << referenceMetrics.sagitta << '\n';
    std::cout << "Curvature score: " << referenceMetrics.curvatureScore << '\n';
    std::cout << "Outputs: " << componentsPath << ", " << skeletonPath << ", " << mainPathsPath << ", " << pathsPath << ", " << vizPath << '\n';
    std::cout << "Temps total: " << elapsedMs << " ms" << std::endl;

    return 0;
}
