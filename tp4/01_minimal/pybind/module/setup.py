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


class BuildExtWithCuda(build_ext):
    """Permet a setuptools de deleguer la compilation des fichiers `.cu` a `nvcc`."""

    def build_extensions(self):
        build_temp = Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)
        compiler_path = None
        nvcc_path = None

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
                object_path = build_temp / f"{source_path.stem}.obj"

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


# `setup(...)` decrit comment construire et installer le mini module Python.
setup(
    # Nom du package Python installe par `pip`.
    name="mini-pybind",
    # Version du package.
    version="0.1.0",
    packages=find_packages(where="."),
    # Liste des extensions natives a compiler.
    ext_modules=[
        # `Extension(...)` decrit un module C/C++ a construire.
        Extension(
            # Nom du module Python final importe ensuite comme
            # `mini_pybind._mini_pybind`.
            "mini_pybind._mini_pybind",
            # Sources du module :
            # - `mini_pybind/bindings.cpp` pour l'interface Python
            # - `mini_pybind/cuda_add.cu` pour l'addition minimale via CUDA
            ["mini_pybind/bindings.cpp", "mini_pybind/cuda_add.cu"],
            # Dossier des en-tetes `pybind11`, necessaires a la compilation.
            include_dirs=[
                pybind11.get_include(),
                str(CUDA_PATH / "include"),
            ],
            library_dirs=[
                str(CUDA_PATH / "lib" / "x64") if os.name == "nt" else str(CUDA_PATH / "lib64")
            ],
            libraries=["cudart"],
            # On indique que l'extension est compilee en C++.
            language="c++",
            extra_compile_args={
                "cxx": ["/EHsc", "/std:c++17"] if os.name == "nt" else ["-std=c++17", "-fPIC"],
            },
        )
    ],
    package_data={"mini_pybind": ["*.pyd", "*.so"]},
    cmdclass={"build_ext": BuildExtWithCuda},
)
