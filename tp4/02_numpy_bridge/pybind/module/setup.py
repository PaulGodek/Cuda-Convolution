from setuptools import Extension, find_packages, setup

import pybind11

# `setup(...)` decrit comment construire et installer le module Python
# de cette partie 2. Ici, il n'y a pas de CUDA : on compile seulement
# le binding `pybind11` et une petite fonction C++.
setup(
    name="tp4-pybind-bridge",
    version="0.1.0",
    packages=find_packages(where="."),
    ext_modules=[
        Extension(
            "tp4_pybind_bridge._tp4_pybind_bridge",
            ["tp4_pybind_bridge/bindings.cpp", "tp4_pybind_bridge/vector_threshold.cpp"],
            include_dirs=[pybind11.get_include()],
            language="c++",
        )
    ],
)
