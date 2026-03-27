import os
import sys
from pathlib import Path

def configure_cuda_runtime():
    # Sous Windows, un module Python natif peut dependre de DLL CUDA.
    # Si `CUDA_PATH` est defini, on ajoute automatiquement son dossier `bin`
    # pour que l'import de `mini_pybind` puisse trouver `cudart64_*.dll`.
    if os.name != "nt" or not hasattr(os, "add_dll_directory"):
        return

    cuda_path = os.environ.get("CUDA_PATH")
    if not cuda_path:
        return

    cuda_bin = Path(cuda_path) / "bin"
    if cuda_bin.is_dir():
        os.add_dll_directory(str(cuda_bin))


configure_cuda_runtime()

try:
    # Prefere le package installe dans l'environnement Python courant.
    from mini_pybind import add_ints_cpp, add_ints_cuda
except ModuleNotFoundError:
    # Fallback pratique pour un lancement sans installation prealable.
    MODULE_ROOT = Path(__file__).resolve().parent / "module"
    if str(MODULE_ROOT) not in sys.path:
        sys.path.insert(0, str(MODULE_ROOT))
    from mini_pybind import add_ints_cpp, add_ints_cuda


print("C++  4 + 7 =", add_ints_cpp(4, 7))
print("CUDA 4 + 7 =", add_ints_cuda(4, 7))

try:
    print("C++  3.5 + 4.2 =", add_ints_cpp(3.5, 4.2))
except TypeError as error:
    print("Erreur avec des float :", error)
