import ctypes
import os
from pathlib import Path


def load_library() -> ctypes.CDLL:
    base = Path(__file__).resolve().parent
    candidates = ["native_lib.dll"] if os.name == "nt" else ["libnative_lib.so", "native_lib.so"]
    for name in candidates:
        path = base / name
        if path.exists():
            return ctypes.CDLL(str(path))
    raise FileNotFoundError("Native library not found")


lib = load_library()
lib.add_ints.argtypes = [ctypes.c_int, ctypes.c_int]
lib.add_ints.restype = ctypes.c_int

result = lib.add_ints(4, 7)
print("4 + 7 =", result)
