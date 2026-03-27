from setuptools import find_packages, setup


setup(
    name="tp4-ctypes-bridge",
    version="0.1.0",
    description="TP4 ctypes bridge from a NumPy 1D array to a native function",
    packages=find_packages(where="."),
    package_dir={"": "."},
    include_package_data=True,
    package_data={"tp4_ctypes_bridge": ["*.dll", "*.so"]},
    install_requires=["numpy"],
)
