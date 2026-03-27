#ifndef TP4_PYBIND_STUDENT_FUNCTIONS_H
#define TP4_PYBIND_STUDENT_FUNCTIONS_H

#include "io.h"
#include <vector>

struct PixelCoord {
    int x = 0;
    int y = 0;
};

Image8 cudaThresholdDepth(const DepthImage& depth, int threshold);
double cudaComputeLargestComponentDiameter(const Image8& mask);
std::vector<PixelCoord> extractComponentPixels(const Image8& mask, int startX, int startY, std::vector<uint8_t>& visited);

Image8 thresholdDepthBuffer(const uint16_t* depthData, int width, int height, int threshold);
double largestComponentDiameterBuffer(const uint8_t* maskData, int width, int height);

#endif // TP4_PYBIND_STUDENT_FUNCTIONS_H
