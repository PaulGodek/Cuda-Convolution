#include "student_functions.h"

#include <stdexcept>

torch::Tensor threshold(torch::Tensor input, float thresholdValue) {
    if (input.is_cuda()) {
        throw std::runtime_error("threshold expects a CPU tensor");
    }

    if (input.dim() != 2) {
        throw std::runtime_error("threshold expects a 2D tensor");
    }

    return threshold_cpu(input, thresholdValue);
}

torch::Tensor normalize(torch::Tensor input) {

    if (input.is_cuda()) {
        throw std::runtime_error("normalize expects a CPU tensor");
    }

    if (input.dim() != 2) {
        throw std::runtime_error("normalize expects a 2D tensor");
    }

    return normalize_cpu(input);
}

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
    m.def("threshold", &threshold, "Threshold a CPU tensor", pybind11::arg("depth"), pybind11::arg("threshold"));
    m.def("normalize", &normalize, "Normalize a CPU tensor");
}
