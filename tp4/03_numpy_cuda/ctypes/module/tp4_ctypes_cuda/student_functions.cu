#include "student_functions.h"

#include "utils_cuda.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <queue>
#include <vector>

namespace {

__global__ void kernelThreshold(const uint16_t* depth, uint8_t* output, int width, int height, int threshold) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) {
        return;
    }

    const int idx = y * width + x;
    output[idx] = static_cast<uint8_t>(depth[idx] > threshold ? 255 : 0);
}

__global__ void kernelComponentDiameter(const PixelCoord* pixels, int count, int* maxDistanceSquared) {
    const unsigned int idx = blockIdx.x * blockDim.x + threadIdx.x;
    const unsigned int totalPairs = static_cast<unsigned int>(count) * static_cast<unsigned int>(count);
    if (idx >= totalPairs) {
        return;
    }

    const int i = static_cast<int>(idx / count);
    const int j = static_cast<int>(idx % count);
    if (j <= i) {
        return;
    }

    const int dx = pixels[j].x - pixels[i].x;
    const int dy = pixels[j].y - pixels[i].y;
    const int distanceSquared = dx * dx + dy * dy;
    atomicMax(maxDistanceSquared, distanceSquared);
}

double cudaComputeComponentDiameter(const std::vector<PixelCoord>& pixels) {
    if (pixels.size() < 2) {
        return 0.0;
    }

    PixelCoord* d_pixels = nullptr;
    int* d_maxDistanceSquared = nullptr;

    CUDA_CHECK(cudaMalloc(&d_pixels, pixels.size() * sizeof(PixelCoord)));
    CUDA_CHECK(cudaMalloc(&d_maxDistanceSquared, sizeof(int)));
    CUDA_CHECK(cudaMemcpy(d_pixels, pixels.data(), pixels.size() * sizeof(PixelCoord), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemset(d_maxDistanceSquared, 0, sizeof(int)));

    const unsigned int totalPairs = static_cast<unsigned int>(pixels.size()) * static_cast<unsigned int>(pixels.size());
    const unsigned int blockSize = 256;
    const unsigned int gridSize = (totalPairs + blockSize - 1) / blockSize;

    kernelComponentDiameter<<<gridSize, blockSize>>>(d_pixels, static_cast<int>(pixels.size()), d_maxDistanceSquared);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    int maxDistanceSquared = 0;
    CUDA_CHECK(cudaMemcpy(&maxDistanceSquared, d_maxDistanceSquared, sizeof(int), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(d_pixels));
    CUDA_CHECK(cudaFree(d_maxDistanceSquared));

    return std::sqrt(static_cast<double>(maxDistanceSquared));
}

} // namespace

Image8 cudaThresholdDepth(const DepthImage& depth, int threshold) {
    Image8 result;
    result.width = depth.width;
    result.height = depth.height;
    result.data.resize(depth.data.size(), 0);

    if (depth.width <= 0 || depth.height <= 0 || depth.data.empty()) {
        return result;
    }

    uint16_t* d_depth = nullptr;
    uint8_t* d_output = nullptr;
    const size_t sizeDepth = depth.data.size() * sizeof(uint16_t);
    const size_t sizeOutput = result.data.size() * sizeof(uint8_t);

    CUDA_CHECK(cudaMalloc(&d_depth, sizeDepth));
    CUDA_CHECK(cudaMalloc(&d_output, sizeOutput));
    CUDA_CHECK(cudaMemcpy(d_depth, depth.data.data(), sizeDepth, cudaMemcpyHostToDevice));

    dim3 block(16, 16);
    dim3 grid((depth.width + block.x - 1) / block.x, (depth.height + block.y - 1) / block.y);
    kernelThreshold<<<grid, block>>>(d_depth, d_output, depth.width, depth.height, threshold);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaMemcpy(result.data.data(), d_output, sizeOutput, cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaFree(d_depth));
    CUDA_CHECK(cudaFree(d_output));
    return result;
}

std::vector<PixelCoord> extractComponentPixels(const Image8& mask, int startX, int startY, std::vector<uint8_t>& visited) {
    std::vector<PixelCoord> pixels;
    if (mask.width <= 0 || mask.height <= 0 || mask.data.empty()) {
        return pixels;
    }
    if (startX < 0 || startX >= mask.width || startY < 0 || startY >= mask.height) {
        return pixels;
    }

    const size_t startIndex = static_cast<size_t>(startY) * mask.width + startX;
    if (mask.data[startIndex] == 0 || visited[startIndex] != 0) {
        return pixels;
    }

    static constexpr int kDx[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    static constexpr int kDy[8] = {-1, -1, -1, 0, 0, 1, 1, 1};

    std::queue<PixelCoord> frontier;
    frontier.push(PixelCoord{startX, startY});
    visited[startIndex] = 1;

    while (!frontier.empty()) {
        const PixelCoord current = frontier.front();
        frontier.pop();
        pixels.push_back(current);

        for (int i = 0; i < 8; ++i) {
            const int nx = current.x + kDx[i];
            const int ny = current.y + kDy[i];
            if (nx < 0 || nx >= mask.width || ny < 0 || ny >= mask.height) {
                continue;
            }

            const size_t neighborIndex = static_cast<size_t>(ny) * mask.width + nx;
            if (mask.data[neighborIndex] == 0 || visited[neighborIndex] != 0) {
                continue;
            }

            visited[neighborIndex] = 1;
            frontier.push(PixelCoord{nx, ny});
        }
    }

    return pixels;
}

double cudaComputeLargestComponentDiameter(const Image8& mask) {
    if (mask.width <= 0 || mask.height <= 0 || mask.data.empty()) {
        return 0.0;
    }

    std::vector<uint8_t> visited(mask.data.size(), 0);
    double largestDiameter = 0.0;

    for (int y = 0; y < mask.height; ++y) {
        for (int x = 0; x < mask.width; ++x) {
            const size_t index = static_cast<size_t>(y) * mask.width + x;
            if (mask.data[index] == 0 || visited[index] != 0) {
                continue;
            }

            const std::vector<PixelCoord> pixels = extractComponentPixels(mask, x, y, visited);
            const double diameter = cudaComputeComponentDiameter(pixels);
            if (diameter > largestDiameter) {
                largestDiameter = diameter;
            }
        }
    }

    return largestDiameter;
}

TP4_EXPORT int tp4ThresholdDepthBuffer(const uint16_t* depthData, int width, int height, int threshold, uint8_t* outputData) {
    if (depthData == nullptr || outputData == nullptr || width <= 0 || height <= 0) {
        return 1;
    }

    DepthImage depth;
    depth.width = width;
    depth.height = height;
    depth.data.assign(depthData, depthData + static_cast<size_t>(width) * height);

    const Image8 result = cudaThresholdDepth(depth, threshold);
    if (result.data.size() != static_cast<size_t>(width) * height) {
        return 2;
    }

    for (size_t i = 0; i < result.data.size(); ++i) {
        outputData[i] = result.data[i];
    }

    return 0;
}

TP4_EXPORT double tp4LargestComponentDiameterBuffer(const uint8_t* maskData, int width, int height) {
    if (maskData == nullptr || width <= 0 || height <= 0) {
        return -1.0;
    }

    Image8 mask;
    mask.width = width;
    mask.height = height;
    mask.data.assign(maskData, maskData + static_cast<size_t>(width) * height);
    return cudaComputeLargestComponentDiameter(mask);
}
