import os
import sys
from pathlib import Path

import numpy as np

# Sous Windows, les modules natifs peuvent dependre des DLL CUDA.
def configure_cuda_runtime():
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
    from tp4_pybind_cuda import largest_component_diameter, threshold_depth
except ModuleNotFoundError:
    MODULE_ROOT = Path(__file__).resolve().parent / "module"
    if str(MODULE_ROOT) not in sys.path:
        sys.path.insert(0, str(MODULE_ROOT))
    from tp4_pybind_cuda import largest_component_diameter, threshold_depth


def main() -> int:
    depth = np.array(
        [
            [0, 1000, 2500, 2600],
            [0, 1200, 2550, 2700],
            [0, 1300, 100, 100],
        ],
        dtype=np.uint16,
    )

    mask = threshold_depth(depth, 1500)
    diameter = largest_component_diameter(mask)

    print("Input depth dtype:", depth.dtype)
    print("Mask dtype:", mask.dtype)
    print("Mask shape:", mask.shape)
    print("Largest component diameter:", float(diameter))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
