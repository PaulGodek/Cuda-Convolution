import os
import sys
from pathlib import Path

import torch

def configure_cuda_runtime():
    # Sous Windows, les modules natifs peuvent dependre des DLL CUDA.
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
    import tp4_torch_cuda
except ModuleNotFoundError:
    MODULE_ROOT = Path(__file__).resolve().parent / "module"
    if str(MODULE_ROOT) not in sys.path:
        sys.path.insert(0, str(MODULE_ROOT))
    import tp4_torch_cuda


def main() -> int:
    if not torch.cuda.is_available():
        print("CUDA not available")
        return 1

    x = torch.tensor(
        [[0.0, 10.0, 20.0], [5.0, 15.0, 25.0]],
        device="cuda",
        dtype=torch.float32,
    )

    y = tp4_torch_cuda.normalize(x)
    print("input device:", x.device)
    print("output device:", y.device)
    print(y)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
