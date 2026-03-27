import os

from setuptools import find_packages, setup

from torch.utils.cpp_extension import BuildExtension, CppExtension


# `setup(...)` decrit ici comment construire l'extension PyTorch/C++
# de la partie 4.
# La vraie nouveaute de cette partie est l'utilisation de :
# - `CppExtension` : pour declarer une extension C++ pour PyTorch ;
# - `BuildExtension` : pour laisser PyTorch preparer le build.
setup(
    # Nom du package installe par `pip`.
    name="tp4-torch-cpu",
    # Version du package.
    version="0.1.0",
    packages=find_packages(where="."),
    # Liste des extensions natives a compiler.
    ext_modules=[
        CppExtension(
            # Nom du module Python final, importe avec `import tp4_torch_cpu`.
            name="tp4_torch_cpu._tp4_torch_cpu",
            # TODO:
            # 1. verifier le nom du module Python ;
            # 2. verifier la liste des fichiers sources a compiler ;
            # 3. ajouter d'eventuels flags si votre machine en a besoin.
            #
            # Ici, la partie vraiment nouvelle par rapport aux parties precedentes est :
            # - on n'utilise plus `Extension(...)` de setuptools ;
            # - on utilise `CppExtension(...)`, fourni par PyTorch ;
            # - c'est lui qui ajoute les bons includes / libs pour `torch::Tensor`.
            # Fichiers sources :
            # - `student_binding.cpp` pour l'interface Python / PyTorch
            # - `student_functions.cpp` pour la logique CPU
            sources=[
                "tp4_torch_cpu/student_binding.cpp",
                "tp4_torch_cpu/student_functions.cpp",
            ],
            # Ici, on reste en C++ uniquement :
            # - pas de `nvcc`
            # - pas de fichier `.cu`
            # - seulement des flags C++
            extra_compile_args=["/std:c++17"] if os.name == "nt" else ["-std=c++17"],
        )
    ],
    # `BuildExtension` fournit la logique de build adaptee aux extensions
    # C++ de PyTorch.
    cmdclass={"build_ext": BuildExtension},
)
