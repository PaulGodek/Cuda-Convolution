import ctypes
import os
from pathlib import Path

import numpy as np


def _candidate_library_names() -> list[str]:
    if os.name == "nt":
        return ["tp4_ctypes_bridge.dll"]
    return ["libtp4_ctypes_bridge.so", "tp4_ctypes_bridge.so"]


def _load_library() -> ctypes.CDLL:
    package_dir = Path(__file__).resolve().parent
    for name in _candidate_library_names():
        candidate = package_dir / name
        if candidate.exists():
            return ctypes.CDLL(str(candidate))
    raise FileNotFoundError("Native bridge library not found next to the Python package")


_LIB = _load_library()
_LIB.tp4ThresholdVectorBuffer.argtypes = [
    ctypes.POINTER(ctypes.c_uint16),
    ctypes.c_int,
    ctypes.c_int,
    ctypes.POINTER(ctypes.c_uint8),
]
_LIB.tp4ThresholdVectorBuffer.restype = ctypes.c_int


def threshold_vector(values: np.ndarray, threshold: int) -> np.ndarray:
    if not isinstance(values, np.ndarray) or values.dtype != np.uint16 or values.ndim != 1:
        raise TypeError("values must be a 1D numpy array of dtype uint16")

    values = np.ascontiguousarray(values)
    output = np.empty(values.shape, dtype=np.uint8)

    status = _LIB.tp4ThresholdVectorBuffer(
        values.ctypes.data_as(ctypes.POINTER(ctypes.c_uint16)),
        int(values.shape[0]),
        int(threshold),
        output.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)),
    )
    if status != 0:
        raise RuntimeError(f"tp4ThresholdVectorBuffer failed with status {status}")
    return output
