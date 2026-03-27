#include <cuda_runtime.h>

namespace {

__global__ void kernelAddInts(const int* a, const int* b, int* out) {
    if (blockIdx.x == 0 && threadIdx.x == 0) {
        out[0] = a[0] + b[0];
    }
}

} // namespace

int add_ints_cuda(int a, int b) {
    // Pointeurs vers la memoire GPU pour les deux entrees et la sortie.
    int* d_a = nullptr;
    int* d_b = nullptr;
    int* d_out = nullptr;

    // Variable CPU qui recevra le resultat final recopie depuis le GPU.
    int result = 0;

    // Allocation de trois entiers sur le GPU :
    // un pour `a`, un pour `b` et un pour le resultat.
    cudaMalloc(&d_a, sizeof(int));
    cudaMalloc(&d_b, sizeof(int));
    cudaMalloc(&d_out, sizeof(int));

    // Copie des deux valeurs d'entree depuis le CPU vers le GPU.
    cudaMemcpy(d_a, &a, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, &b, sizeof(int), cudaMemcpyHostToDevice);

    // Lancement d'un kernel minuscule avec 1 bloc de 1 thread.
    // Ce thread lit `a` et `b` sur le GPU, puis ecrit leur somme dans `d_out`.
    kernelAddInts<<<1, 1>>>(d_a, d_b, d_out);

    // Copie du resultat depuis le GPU vers la variable CPU `result`.
    cudaMemcpy(&result, d_out, sizeof(int), cudaMemcpyDeviceToHost);

    // Liberation de la memoire GPU allouee plus haut.
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_out);

    // Retour du resultat final au code C++ / Python.
    return result;
}
