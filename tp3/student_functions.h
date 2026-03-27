#ifndef STUDENT_FUNCTIONS_H
#define STUDENT_FUNCTIONS_H

#include "../io.h"
#include <cstdint>
#include <string>

struct CropInfo {
    int originalWidth = 0;
    int originalHeight = 0;
    int cropX = 0;
    int cropY = 0;
    int cropW = 0;
    int cropH = 0;
};

struct PlaneModel {
    double a = 0.0;
    double b = 0.0;
    double c = -1.0;
    double d = 0.0;
};

bool loadCropInfo(const std::string& path, CropInfo& info);
DepthImage cropDepth(const DepthImage& depth, const CropInfo& info);
Image8 reconstructFullMask(const Image8& croppedMask, const CropInfo& info);

PlaneModel fitPlaneRansac(const DepthImage& depth, const Image8& initialMask,
                          int iterations, double inlierThreshold, bool& ok);
DepthImage flattenDepthAgainstPlane(const DepthImage& depth, const PlaneModel& plane);
Image8 thresholdStrongSeeds(const DepthImage& flattened, uint16_t seedThreshold);
Image8 regionGrowMask(const DepthImage& flattened, const Image8& seedMask,
                      uint16_t growDiff, uint16_t planeStopThreshold);

int countOverlap(const Image8& mask1, const Image8& mask2);
int countUnion(const Image8& mask1, const Image8& mask2);
double computeSimpleScore(const Image8& mask1, const Image8& mask2);
double computeIoU(const Image8& mask1, const Image8& mask2);

Image8 makeDepthPreview(const DepthImage& depth, bool invert);

#endif // STUDENT_FUNCTIONS_H
