import os

from setuptools import find_packages, setup

from torch.utils.cpp_extension import BuildExtension, CUDAExtension


# `setup(...)` decrit ici comment construire l'extension PyTorch/CUDA
# de la partie 5.
setup(
    # Nom du package installe par `pip`.
    name="tp4-torch-cuda",
    # Version du package.
    version="0.1.0",
    packages=find_packages(where="."),
    # Liste des extensions natives a compiler.
    ext_modules=[
        CUDAExtension(
            # Nom du module Python final, importe avec `import tp4_torch_cuda`.
            name="tp4_torch_cuda._tp4_torch_cuda",
            # TODO:
            # 1. verifier le nom du module Python ;
            # 2. verifier la liste des fichiers sources a compiler ;
            # 3. ajouter d'eventuels flags si votre machine en a besoin.
            # Cette partie suppose une installation PyTorch avec support CUDA.
            # Une build CPU-only de `torch` ne peut pas compiler / charger
            # cette extension CUDA.
            # Fichiers sources :
            # - `student_binding.cpp` pour l'interface Python / PyTorch
            # - `student_functions.cu` pour la logique CUDA
            sources=[
                "tp4_torch_cuda/student_binding.cpp",
                "tp4_torch_cuda/student_functions.cu",
            ],
            extra_compile_args={
                "cxx": ["/std:c++17"] if os.name == "nt" else ["-std=c++17"],
                "nvcc": [],
            },
        )
    ],
    # `BuildExtension` fournit la logique de build adaptee aux extensions
    # C++ / CUDA de PyTorch.
    cmdclass={"build_ext": BuildExtension},
)
