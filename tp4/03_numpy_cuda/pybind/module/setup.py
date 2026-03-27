import os
import shutil
from pathlib import Path

from setuptools import Extension, find_packages, setup
from setuptools.command.build_ext import build_ext
from setuptools._distutils._msvccompiler import MSVCCompiler

import pybind11


if os.name == "nt":
    CUDA_PATH = Path(os.environ.get("CUDA_PATH", r"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.4"))
else:
    CUDA_PATH = Path(os.environ.get("CUDA_PATH", "/usr/local/cuda"))

PROJECT_ROOT = Path(__file__).resolve().parents[4]


class BuildExtWithCuda(build_ext):
    """Compile les fichiers `.cu` avec `nvcc`, sous Windows comme sous Linux."""

    def build_extensions(self):
        build_temp = Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)

        if os.name == "nt":
            msvc_compiler = MSVCCompiler()
            msvc_compiler.initialize()
            compiler_path = Path(msvc_compiler.cc)
            nvcc_path = CUDA_PATH / "bin" / "nvcc.exe"
        else:
            compiler_name = os.environ.get("CXX", "c++")
            resolved_compiler = shutil.which(compiler_name)
            if resolved_compiler is None:
                raise RuntimeError(f"C++ compiler not found: {compiler_name}")
            compiler_path = Path(resolved_compiler)

            resolved_nvcc = shutil.which("nvcc")
            if resolved_nvcc is None:
                resolved_nvcc = str(CUDA_PATH / "bin" / "nvcc")
            nvcc_path = Path(resolved_nvcc)

        for extension in self.extensions:
            cuda_sources = [source for source in extension.sources if Path(source).suffix == ".cu"]
            cpp_sources = [source for source in extension.sources if Path(source).suffix != ".cu"]

            cuda_objects = []
            for source in cuda_sources:
                source_path = Path(source)
                object_suffix = ".obj" if os.name == "nt" else ".o"
                object_path = build_temp / f"{source_path.stem}{object_suffix}"

                nvcc_command = [
                    str(nvcc_path),
                    "-c",
                    str(source_path),
                    "-o",
                    str(object_path),
                    "-I",
                    pybind11.get_include(),
                    "-I",
                    str(CUDA_PATH / "include"),
                    "-I",
                    str(PROJECT_ROOT),
                ]

                if os.name == "nt":
                    nvcc_command.extend(
                        [
                            "-ccbin",
                            str(compiler_path.parent),
                            "-O2",
                            "-Xcompiler",
                            "/EHsc,/MD",
                        ]
                    )
                else:
                    nvcc_command.extend(
                        [
                            "-ccbin",
                            str(compiler_path),
                            "-O2",
                            "-Xcompiler",
                            "-fPIC",
                        ]
                    )

                self.spawn(nvcc_command)
                cuda_objects.append(str(object_path))

            extension.sources = cpp_sources
            extension.extra_objects = list(getattr(extension, "extra_objects", [])) + cuda_objects

            if isinstance(extension.extra_compile_args, dict):
                extension.extra_compile_args = extension.extra_compile_args.get("cxx", [])

        super().build_extensions()


# `setup(...)` decrit ici comment construire le package Python de la partie 3
# et comment compiler le module natif `pybind11`.
setup(
    # Nom du package installe par `pip`.
    name="tp4-pybind-cuda",
    # Version du package.
    version="0.1.0",
    # Petite description visible dans les metadonnees du package.
    description="TP4 pybind11 package exposing CUDA functions",
    # Recherche les packages Python dans le dossier courant.
    packages=find_packages(where="."),
    # Liste des extensions natives a compiler.
    ext_modules=[
        Extension(
            # Nom du module natif final, importe ensuite comme
            # `tp4_pybind_cuda._tp4_pybind_cuda`.
            "tp4_pybind_cuda._tp4_pybind_cuda",
            # Fichiers sources :
            # - `tp4_pybind_cuda/student_binding.cpp` pour le pont Python <-> C++
            # - `tp4_pybind_cuda/student_functions.cu` pour la logique CUDA
            [
                "tp4_pybind_cuda/student_binding.cpp",
                "tp4_pybind_cuda/student_functions.cu",
            ],
            # En-tetes `pybind11` necessaires a la compilation du binding.
            include_dirs=[
                pybind11.get_include(),
                str(CUDA_PATH / "include"),
                str(PROJECT_ROOT),
            ],
            library_dirs=[
                str(CUDA_PATH / "lib" / "x64") if os.name == "nt" else str(CUDA_PATH / "lib64")
            ],
            libraries=["cudart"],
            # Le module est compile en C++.
            language="c++",
            extra_compile_args={
                "cxx": ["/EHsc", "/std:c++17"] if os.name == "nt" else ["-std=c++17", "-fPIC"],
            },
        )
    ],
    # Autorise l'inclusion de fichiers supplementaires dans le package installe.
    include_package_data=True,
    # Indique que le package peut contenir un module natif deja compile
    # (`.pyd` sous Windows, `.so` sous Linux).
    package_data={"tp4_pybind_cuda": ["*.pyd", "*.so"]},
    # Dependances Python minimales pour utiliser l'API.
    install_requires=["numpy"],
    cmdclass={"build_ext": BuildExtWithCuda},
)
