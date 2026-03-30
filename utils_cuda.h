#ifndef UTILS_CUDA_H
#define UTILS_CUDA_H

#include <cuda_runtime.h>
#include <iostream>

// Vérification d'erreurs CUDA
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            std::cerr << "Erreur CUDA : " << cudaGetErrorString(err) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            exit(1); \
        } \
    } while (0)

// Allocation mémoire GPU
template<typename T>
T* cudaMallocManagedChecked(size_t size) {
    T* ptr;
    CUDA_CHECK(cudaMallocManaged(&ptr, size * sizeof(T)));
    return ptr;
}

// Copie vers GPU
template<typename T>
void cudaMemcpyToDeviceChecked(T* d_ptr, const T* h_ptr, size_t size) {
    CUDA_CHECK(cudaMemcpy(d_ptr, h_ptr, size * sizeof(T), cudaMemcpyHostToDevice));
}

// Copie depuis GPU
template<typename T>
void cudaMemcpyToHostChecked(T* h_ptr, const T* d_ptr, size_t size) {
    CUDA_CHECK(cudaMemcpy(h_ptr, d_ptr, size * sizeof(T), cudaMemcpyDeviceToHost));
}

// Synchronisation
inline void cudaDeviceSynchronizeChecked() {
    CUDA_CHECK(cudaDeviceSynchronize());
}

// Libération mémoire
template<typename T>
void cudaFreeChecked(T* ptr) {
    CUDA_CHECK(cudaFree(ptr));
}

#endif // UTILS_CUDA_H
