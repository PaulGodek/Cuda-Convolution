#ifndef STUDENT_CONFIG_H
#define STUDENT_CONFIG_H

struct TP3Config {
    const char* defaultDepthPath = "../data/D/pointCloud_data_2026-02-19_12-03-32_depth.tiff";
    const char* defaultInitialMaskPath = "../tp1-bis/mask_final.png";
    const char* defaultCropInfoPath = "../tp1-bis/mask_final_crop.txt";
    const char* defaultGroundTruthPath = "../data/GT/pointCloud_data_2026-02-19_12-03-32_rgb_semantic_mask.png";
    int defaultSeedThreshold = 50;
    int defaultGrowDiff = 1;
    int defaultPlaneStopThreshold = 1;
    int defaultRansacIterations = 400;
    double defaultRansacThreshold = 20.0;
};

inline constexpr TP3Config kStudentConfig{};

#endif // STUDENT_CONFIG_H
