#include <pybind11/pybind11.h>

// Version purement C++.
int add_ints_cpp(int a, int b) {
    return a + b;
}

// Version CUDA definie dans `cuda_add.cu`.
int add_ints_cuda(int a, int b);

PYBIND11_MODULE(_mini_pybind, m) {
    m.doc() = "Minimal pybind11 example with one C++ path and one CUDA path";
    // `pybind11::arg("a")` et `pybind11::arg("b")` donnent un nom Python
    // aux parametres de la fonction. Cela permet d'appeler la fonction
    // proprement depuis Python, par exemple `add_ints_cpp(a=4, b=7)`.
    m.def("add_ints_cpp", &add_ints_cpp, pybind11::arg("a"), pybind11::arg("b"));
    m.def("add_ints_cuda", &add_ints_cuda, pybind11::arg("a"), pybind11::arg("b"));
}
