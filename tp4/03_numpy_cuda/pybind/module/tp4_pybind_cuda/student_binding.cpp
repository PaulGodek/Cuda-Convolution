#include "student_functions.h"

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <cstddef>
#include <cstring>
#include <stdexcept>

// Ce fichier ne contient pas l'algorithme CUDA.
// Son role est seulement de faire le pont entre :
// - des objets Python / NumPy ;
// - et des fonctions C++ / CUDA qui travaillent sur des buffers bruts.

pybind11::array_t<uint8_t> threshold_depth_binding(
    pybind11::array_t<uint16_t, pybind11::array::c_style | pybind11::array::forcecast> depth,
    int threshold) {
    // TODO:
    // 1. verifier que depth est bien 2D ;
    // 2. recuperer height, width et le pointeur vers les donnees ;
    // 3. appeler thresholdDepthBuffer ;
    // 4. creer et retourner un py::array_t<uint8_t> de meme forme.
    //
    // Etape 1 :
    // `depth` est un tableau NumPy recu depuis Python.
    // `request()` demande a pybind11 sa description "bas niveau" :
    // - combien il y a de dimensions ;
    // - quelle est sa forme ;
    // - ou se trouvent les donnees en memoire.

}

double largest_component_diameter_binding(
    pybind11::array_t<uint8_t, pybind11::array::c_style | pybind11::array::forcecast> mask) {
    // TODO:
    // 1. verifier que mask est bien 2D ;
    // 2. recuperer height, width et le pointeur vers les donnees ;
    // 3. appeler largestComponentDiameterBuffer ;
    // 4. retourner le float.
    //
    // Meme idee que pour `threshold_depth_binding`, mais ici la sortie
    // n'est pas un tableau : c'est un simple nombre.

}

PYBIND11_MODULE(_tp4_pybind_cuda, m) {
    m.doc() = "TP4 CUDA bindings with pybind11";

    // TODO:
    // 1. exposer la fonction Python `threshold_depth(...)` ;
    // 2. exposer la fonction Python `largest_component_diameter(...)`.
    //
    // Indice :
    // reprenez la meme idee qu'en partie 1 avec `m.def(...)`.
    throw std::runtime_error("Not implemented");
}
