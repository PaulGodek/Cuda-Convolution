import sys
from pathlib import Path

import torch

try:
    import tp4_torch_cpu
except ModuleNotFoundError:
    MODULE_ROOT = Path(__file__).resolve().parent / "module"
    if str(MODULE_ROOT) not in sys.path:
        sys.path.insert(0, str(MODULE_ROOT))
    import tp4_torch_cpu


def main() -> int:
    x = torch.tensor(
        [[0.0, 10.0, 20.0], [5.0, 15.0, 25.0]],
        dtype=torch.float32,
    )

    mask = tp4_torch_cpu.threshold(x, 12.0)
    y = tp4_torch_cpu.normalize(x)
    print("input device:", x.device)
    print("threshold output dtype:", mask.dtype)
    print(mask)
    print("output device:", y.device)
    print(y)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
