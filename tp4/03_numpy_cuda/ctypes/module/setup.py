from setuptools import find_packages, setup


setup(
    name="tp4-ctypes-cuda",
    version="0.1.0",
    description="TP4 ctypes package exposing CUDA functions",
    packages=find_packages(where="."),
    package_dir={"": "."},
    include_package_data=True,
    package_data={"tp4_ctypes_cuda": ["*.dll", "*.so"]},
    install_requires=["numpy"],
)
