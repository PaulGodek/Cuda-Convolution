#include "student_functions.h"

#include <ATen/cuda/CUDAContext.h>
#include <ATen/ops/amax.h>
#include <ATen/ops/amin.h>
#include <cuda_runtime.h>

namespace {

__global__ void kernelNormalize(const float* input, float* output, int width, int height, float minValue, float maxValue) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= width || y >= height) {
        return;
    }

    const int idx = y * width + x;
    const float denom = maxValue - minValue;
    if (denom <= 0.0f) {
        output[idx] = 0.0f;
        return;
    }

    output[idx] = (input[idx] - minValue) / denom;
}

} // namespace

torch::Tensor normalize_cuda(torch::Tensor input) {
    // TODO:
    // 1. convertir / forcer le tenseur en float32 si besoin ;
    // 2. calculer min et max avec l'API torch C++ ;
    // 3. allouer un tenseur de sortie ;
    // 4. lancer kernelNormalize ;
    // 5. retourner le tenseur resultat.

    return output;
}
