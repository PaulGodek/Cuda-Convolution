#ifndef TP4_CTYPES_CUDA_STUDENT_FUNCTIONS_H
#define TP4_CTYPES_CUDA_STUDENT_FUNCTIONS_H

#include "io.h"
#include <vector>

struct PixelCoord {
    int x = 0;
    int y = 0;
};

Image8 cudaThresholdDepth(const DepthImage& depth, int threshold);
double cudaComputeLargestComponentDiameter(const Image8& mask);
std::vector<PixelCoord> extractComponentPixels(const Image8& mask, int startX, int startY, std::vector<uint8_t>& visited);

#if defined(_WIN32)
#define TP4_EXPORT extern "C" __declspec(dllexport)
#else
#define TP4_EXPORT extern "C"
#endif

TP4_EXPORT int tp4ThresholdDepthBuffer(const uint16_t* depthData, int width, int height, int threshold, uint8_t* outputData);
TP4_EXPORT double tp4LargestComponentDiameterBuffer(const uint8_t* maskData, int width, int height);

#endif // TP4_CTYPES_CUDA_STUDENT_FUNCTIONS_H
