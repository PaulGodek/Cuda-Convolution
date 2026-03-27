#include "vector_threshold.h"

#include <cstddef>

std::vector<uint8_t> threshold_vector_cpp(const uint16_t* values, int length, int threshold) {
    std::vector<uint8_t> mask(static_cast<size_t>(length), 0);

    for (int i = 0; i < length; ++i) {
        mask[static_cast<size_t>(i)] = (values[i] >= threshold) ? 255 : 0;
    }

    return mask;
}
