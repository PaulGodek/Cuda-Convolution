#include "../io.h"
#include "student_config.h"
#include "student_functions.h"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

int parseOrDefault(const char* text, int fallback) {
    if (text == nullptr) {
        return fallback;
    }
    return std::atoi(text);
}

double parseOrDefaultDouble(const char* text, double fallback) {
    if (text == nullptr) {
        return fallback;
    }
    return std::atof(text);
}

} // namespace

int main(int argc, char* argv[]) {
    const std::string depthPath = (argc > 1) ? argv[1] : kStudentConfig.defaultDepthPath;
    const std::string initialMaskPath = (argc > 2) ? argv[2] : kStudentConfig.defaultInitialMaskPath;
    const std::string cropInfoPath = (argc > 3) ? argv[3] : kStudentConfig.defaultCropInfoPath;
    const std::string groundTruthPath = (argc > 4) ? argv[4] : kStudentConfig.defaultGroundTruthPath;
    const uint16_t seedThreshold = static_cast<uint16_t>(parseOrDefault((argc > 5) ? argv[5] : nullptr, kStudentConfig.defaultSeedThreshold));
    const uint16_t growDiff = static_cast<uint16_t>(parseOrDefault((argc > 6) ? argv[6] : nullptr, kStudentConfig.defaultGrowDiff));
    const uint16_t planeStopThreshold = static_cast<uint16_t>(parseOrDefault((argc > 7) ? argv[7] : nullptr, kStudentConfig.defaultPlaneStopThreshold));
    const int ransacIterations = parseOrDefault((argc > 8) ? argv[8] : nullptr, kStudentConfig.defaultRansacIterations);
    const double ransacThreshold = parseOrDefaultDouble((argc > 9) ? argv[9] : nullptr, kStudentConfig.defaultRansacThreshold);

    std::cout << "Depth image: " << depthPath << std::endl;
    std::cout << "Initial mask: " << initialMaskPath << std::endl;
    std::cout << "Crop info: " << cropInfoPath << std::endl;
    std::cout << "Ground truth: " << groundTruthPath << std::endl;

    const auto start = std::chrono::high_resolution_clock::now();

    DepthImage fullDepth = loadDepth(depthPath);
    Image8 initialMask = loadImage(initialMaskPath);
    Image8 gtMask = loadImage(groundTruthPath);
    CropInfo cropInfo;

    if (fullDepth.data.empty() || initialMask.data.empty() || gtMask.data.empty()) {
        std::cerr << "Erreur : impossible de charger une ou plusieurs entrees." << std::endl;
        return 1;
    }

    if (!loadCropInfo(cropInfoPath, cropInfo)) {
        std::cerr << "Erreur : impossible de charger les informations de crop." << std::endl;
        return 1;
    }

    DepthImage croppedDepth = cropDepth(fullDepth, cropInfo);
    if (croppedDepth.data.empty()) {
        std::cerr << "Erreur : impossible de recadrer l'image de profondeur." << std::endl;
        return 1;
    }

    if (croppedDepth.width != initialMask.width || croppedDepth.height != initialMask.height) {
        std::cerr << "Erreur : le crop de profondeur ne correspond pas aux dimensions du masque initial." << std::endl;
        return 1;
    }

    bool planeOk = false;
    const PlaneModel plane = fitPlaneRansac(croppedDepth, initialMask, ransacIterations, ransacThreshold, planeOk);
    if (!planeOk) {
        std::cerr << "Erreur : impossible d'ajuster un plan de fond par RANSAC." << std::endl;
        return 1;
    }

    DepthImage flattened = flattenDepthAgainstPlane(croppedDepth, plane);
    Image8 seedMask = thresholdStrongSeeds(flattened, seedThreshold);
    Image8 refinedMask = regionGrowMask(flattened, seedMask, growDiff, planeStopThreshold);
    Image8 fullMask = reconstructFullMask(refinedMask, cropInfo);

    if (fullMask.data.empty()) {
        std::cerr << "Erreur : impossible de reconstruire le masque pleine taille." << std::endl;
        return 1;
    }

    if (fullMask.width != gtMask.width || fullMask.height != gtMask.height) {
        std::cerr << "Erreur : le masque final et la ground truth n'ont pas les memes dimensions." << std::endl;
        return 1;
    }

    saveDepth(flattened, "flattened_depth.bin");
    saveImage(makeDepthPreview(flattened, true), "flattened_preview.bmp");
    saveImage(seedMask, "region_seed_mask.bmp");
    saveImage(refinedMask, "region_grown_mask.bmp");
    saveImage(fullMask, "region_grown_full_mask.bmp");

    const int overlap = countOverlap(fullMask, gtMask);
    const int unionCount = countUnion(fullMask, gtMask);
    const double coverage = computeSimpleScore(fullMask, gtMask);
    const double iou = computeIoU(fullMask, gtMask);

    const auto end = std::chrono::high_resolution_clock::now();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Plan estime : "
              << plane.a << "x + "
              << plane.b << "y + "
              << plane.c << "z + "
              << plane.d << " = 0" << std::endl;
    std::cout << "Seed threshold : " << seedThreshold << std::endl;
    std::cout << "Grow diff : " << growDiff << std::endl;
    std::cout << "Plane stop threshold : " << planeStopThreshold << std::endl;
    std::cout << "Overlap : " << overlap << std::endl;
    std::cout << "Union : " << unionCount << std::endl;
    std::cout << "Coverage (score simple TP2) : " << coverage << std::endl;
    std::cout << "IoU : " << iou << std::endl;
    std::cout << "Temps total : " << elapsedMs << " ms" << std::endl;

    return 0;
}
