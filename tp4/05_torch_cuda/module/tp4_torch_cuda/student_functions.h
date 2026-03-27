#ifndef TP4_TORCH_CUDA_STUDENT_FUNCTIONS_H
#define TP4_TORCH_CUDA_STUDENT_FUNCTIONS_H

#include <torch/extension.h>

torch::Tensor normalize_cuda(torch::Tensor input);

#endif // TP4_TORCH_CUDA_STUDENT_FUNCTIONS_H
