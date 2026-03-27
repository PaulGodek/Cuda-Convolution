import sys
from pathlib import Path

import numpy as np

try:
    from tp4_ctypes_bridge import threshold_vector
except ModuleNotFoundError:
    MODULE_ROOT = Path(__file__).resolve().parent / "module"
    if str(MODULE_ROOT) not in sys.path:
        sys.path.insert(0, str(MODULE_ROOT))
    from tp4_ctypes_bridge import threshold_vector

# Meme pont pedagogique qu'en pybind11, mais via `ctypes` :
# un vecteur NumPy 1D est envoye a une bibliotheque native,
# puis le resultat revient sous forme de tableau NumPy.
values = np.array([0, 500, 1200, 2500], dtype=np.uint16)
mask = threshold_vector(values, 1000)
print("Vector input :", values)
print("Vector mask  :", mask)
