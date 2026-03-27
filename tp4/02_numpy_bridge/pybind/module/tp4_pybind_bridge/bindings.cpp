#include "vector_threshold.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

// Ce fichier ne contient pas l'algorithme lui-meme.
// Son role est seulement de faire le pont entre :
// - un tableau NumPy recu depuis Python ;
// - une fonction C++ qui travaille sur des donnees brutes ;
// - puis un nouveau tableau NumPy renvoye a Python.

pybind11::array_t<uint8_t> threshold_vector_binding(
    pybind11::array_t<uint16_t, pybind11::array::c_style | pybind11::array::forcecast> values,
    int threshold) {
    // Etape 1 :
    // `values` est un tableau NumPy 1D recu depuis Python.
    // `request()` demande a pybind11 sa description :
    // - le nombre de dimensions ;
    // - la forme ;
    // - l'adresse des donnees en memoire.
    const pybind11::buffer_info inputBuffer = values.request();
    if (inputBuffer.ndim != 1) {
        throw std::runtime_error("values must be a 1D array");
    }

    // Etape 2 :
    // `shape[0]` est la longueur du vecteur 1D.
    const int length = static_cast<int>(inputBuffer.shape[0]);

    // Etape 3 :
    // `ptr` est un pointeur brut vers les donnees du tableau NumPy.
    const auto* inputData = static_cast<const uint16_t*>(inputBuffer.ptr);

    // Etape 4 :
    // on appelle une fonction C++ simple qui lit ce pointeur et construit
    // un resultat sous forme de `std::vector<uint8_t>`.
    const std::vector<uint8_t> outputValues = threshold_vector_cpp(inputData, length, threshold);

    // Etape 5 :
    // on cree un nouveau tableau NumPy 1D pour le retour Python.
    // Au moment de sa creation, ce tableau existe deja,
    // mais il ne contient pas encore le resultat.
    pybind11::array_t<uint8_t> output(length);

    // Etape 6 :
    // on demande a pybind11 ou se trouve la memoire du tableau de sortie.
    pybind11::buffer_info outputBuffer = output.request();
    auto* outputData = static_cast<uint8_t*>(outputBuffer.ptr);

    // Etape 7 :
    // on remplit le tableau NumPy de sortie case par case
    // avec les valeurs calculees en C++.
    for (int i = 0; i < length; ++i) {
        outputData[i] = outputValues[static_cast<size_t>(i)];
    }

    return output;
}

PYBIND11_MODULE(_tp4_pybind_bridge, m) {
    m.doc() = "pybind11 bridge from a NumPy 1D array to a C++ function and back";
    // Cette ligne cree une fonction Python `threshold_vector(...)`
    // qui appellera la fonction C++ `threshold_vector_binding(...)`.
    m.def("threshold_vector", &threshold_vector_binding, pybind11::arg("values"), pybind11::arg("threshold"));
}
