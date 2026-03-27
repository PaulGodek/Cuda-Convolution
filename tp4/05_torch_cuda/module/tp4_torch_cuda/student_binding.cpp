#include "student_functions.h"

#include <stdexcept>

torch::Tensor normalize(torch::Tensor input) {

    if (!input.is_cuda()) {
        throw std::runtime_error("normalize expects a CUDA tensor");
    }

    if (input.dim() != 2) {
        throw std::runtime_error("normalize expects a 2D tensor");
    }

    return normalize_cuda(input);
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
    m.def("normalize", &normalize, "Normalize a CUDA tensor");
}
