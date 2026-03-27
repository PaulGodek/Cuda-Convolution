#ifndef TP4_TORCH_CPU_STUDENT_FUNCTIONS_H
#define TP4_TORCH_CPU_STUDENT_FUNCTIONS_H

#include <torch/extension.h>

torch::Tensor threshold_cpu(torch::Tensor input, float threshold);
torch::Tensor normalize_cpu(torch::Tensor input);

#endif // TP4_TORCH_CPU_STUDENT_FUNCTIONS_H
