import ctypes
import os
from pathlib import Path

import numpy as np


def _candidate_library_names() -> list[str]:
    if os.name == "nt":
        return ["tp4_ctypes_cuda.dll"]
    return ["libtp4_ctypes_cuda.so", "tp4_ctypes_cuda.so"]


def _load_library() -> ctypes.CDLL:
    package_dir = Path(__file__).resolve().parent
    for name in _candidate_library_names():
        candidate = package_dir / name
        if candidate.exists():
            return ctypes.CDLL(str(candidate))
    raise FileNotFoundError("CUDA library not found next to the Python package")


_LIB = _load_library()

# TODO:
# Declarer les signatures ctypes des deux fonctions exportees :
# - tp4ThresholdDepthBuffer
# - tp4LargestComponentDiameterBuffer


def threshold_depth(depth: np.ndarray, threshold: int) -> np.ndarray:
    # TODO:
    # 1. verifier que depth est un tableau 2D numpy de type uint16 ;
    # 2. allouer un tableau output de type uint8 avec la meme forme ;
    # 3. appeler tp4ThresholdDepthBuffer avec des pointeurs ctypes ;
    # 4. retourner le masque numpy.
    raise NotImplementedError


def largest_component_diameter(mask: np.ndarray) -> float:
    # TODO:
    # 1. verifier que mask est un tableau 2D numpy de type uint8 ;
    # 2. appeler tp4LargestComponentDiameterBuffer ;
    # 3. retourner un float Python.
    raise NotImplementedError
