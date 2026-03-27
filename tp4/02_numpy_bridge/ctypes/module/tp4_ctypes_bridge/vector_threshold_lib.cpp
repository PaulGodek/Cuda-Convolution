#include "vector_threshold_lib.h"

#include <cstddef>

TP4_BRIDGE_EXPORT int tp4ThresholdVectorBuffer(
    const uint16_t* values,
    int length,
    int threshold,
    uint8_t* outputData) {
    if (values == nullptr || outputData == nullptr || length < 0) {
        return 1;
    }

    for (int i = 0; i < length; ++i) {
        outputData[static_cast<size_t>(i)] = (values[i] >= threshold) ? 255 : 0;
    }

    return 0;
}
