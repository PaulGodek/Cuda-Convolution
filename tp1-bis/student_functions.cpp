#include "student_functions.h"
#include <algorithm>
#include <iostream>

void normalizeDepth(DepthImage& depth) {
    if (depth.data.empty()) {
        return;
    }

    // Find the minimum and maximum depth values in the image.
    uint16_t min = depth.data[0];
    uint16_t max = depth.data[0];
    for (uint16_t value : depth.data) {
        if (value < min) min = value;
        if (value > max) max = value;
    }

    // Apply the linear normalization formula to map values to [0, 65535].
    for (size_t i = 0; i < depth.data.size(); i++) {
        depth.data[i] = 0 + (depth.data[i] - min) / (max - min) * (65535 - 0);
        // Thanks https://stackoverflow.com/a/28916570
    }
}

void invertDepth(DepthImage& depth) {
    for (auto& value : depth.data) {
        value = static_cast<uint16_t>(65535u - value);
    }
}

Image8 thresholdDepth(const DepthImage& depth, uint16_t threshold) {
    Image8 result;
    result.width = depth.width;
    result.height = depth.height;
    result.data.resize(depth.data.size(), 0);

    for (size_t i = 0; i < depth.data.size(); i++) { // if > threshold, 255, else 0
        if (depth.data[i] < threshold) result.data[i] = 0;
        else result.data[i] = 255;
    }

    return result;
}

Image8 connex8ComparaisonMask(const Image8& mask, uint8_t comparative) {
    Image8 result;
    result.width = mask.width;
    result.height = mask.height;
    result.data.resize(mask.data.size(), 0);

    // Apply 8 connex modif
    for (size_t i = 0; i < mask.height; i++) {
        for (size_t j = 0; j < mask.width; j++) {

            // Set if you can check the value on the right, left, up or down
            bool canGoLeft = j > 0;
            bool canGoRight = j < mask.width - 1;
            bool canGoHigher = i > 0;
            bool canGoLower = i < mask.height - 1;

            // Check if the neighboors pass the comparaison 
            uint8_t value = mask.data[j + (i * mask.width)];
            bool changeValue = false;
            if (canGoHigher) {
                if (mask.data[j + ((i - 1) * mask.width)] == comparative) changeValue = true;
                if (canGoRight) {
                    if (mask.data[j + 1 + ((i - 1) * mask.width)] == comparative) changeValue = true;
                }
                if (canGoLeft) {
                    if (mask.data[j - 1 + ((i - 1) * mask.width)] == comparative) changeValue = true;
                }
            }
            if (canGoLower) {
                if (mask.data[j + ((i + 1) * mask.width)] == comparative) changeValue = true;
                if (canGoRight) {
                    if (mask.data[j + 1 + ((i + 1) * mask.width)] == comparative) changeValue = true;
                }
                if (canGoLeft) {
                    if (mask.data[j - 1 + ((i + 1) * mask.width)] == comparative) changeValue = true;
                }
            }
            if (canGoRight) {
                if (mask.data[j + 1 + (i * mask.width)] == comparative) changeValue = true;
            } 
            if (canGoLeft) {
                if (mask.data[j - 1 + (i * mask.width)] == comparative) changeValue = true;
            }

            if (changeValue) {
                result.data[j + (i * mask.width)] = comparative;
            }
            else result.data[j + (i * mask.width)] = value;
        }
    }

    return result;
}

// Résultat mieux de GPT-sama : 
// ...existing code...
// Image8 connex8ComparaisonMask(const Image8& mask, uint8_t comparative) {
//     Image8 result;
//     result.width = mask.width;
//     result.height = mask.height;
//     result.data.resize(mask.data.size(), 0);

//     if (mask.width <= 0 || mask.height <= 0) return result;

//     const int w = mask.width;
//     const int h = mask.height;

//     for (int i = 0; i < h; ++i) {
//         for (int j = 0; j < w; ++j) {
//             const int idx = j + i * w;
//             const uint8_t value = mask.data[idx];
//             bool changeValue = false;

//             // check 8 neighbors
//             for (int di = -1; di <= 1 && !changeValue; ++di) {
//                 for (int dj = -1; dj <= 1 && !changeValue; ++dj) {
//                     if (di == 0 && dj == 0) continue;
//                     const int ni = i + di;
//                     const int nj = j + dj;
//                     if (ni < 0 || ni >= h || nj < 0 || nj >= w) continue;
//                     const int nidx = nj + ni * w;
//                     if (mask.data[nidx] == comparative) changeValue = true;
//                 }
//             }

//             result.data[idx] = changeValue ? comparative : value;
//         }
//     }

//     return result;
// }

Image8 erodeMask(const Image8& mask) {
    return connex8ComparaisonMask(mask, 0);
}

Image8 dilateMask(const Image8& mask) {
    return connex8ComparaisonMask(mask, 255);
}

Image8 openMask(const Image8& mask) {
    // Opening = erosion followed by dilation.
    Image8 result = mask;
    result = erodeMask(result);
    result = dilateMask(result);

    return result;
}

Image8 closeMask(const Image8& mask) {
    // Closing = dilation followed by erosion.
    Image8 result = mask;
    result = dilateMask(result);
    result = erodeMask(result);

    return result;
}

Image8 cropMask(const Image8& mask, int cropX, int cropY, int cropW, int cropH) {
    Image8 result;
    result.width = 0;
    result.height = 0;

    if (mask.width <= 0 || mask.height <= 0 || mask.data.empty()) {
        return result;
    }

    if (cropX >= mask.width || cropY >= mask.height) {
        return result;
    }

    const int effectiveWidth = std::min(cropW, mask.width - cropX);
    const int effectiveHeight = std::min(cropH, mask.height - cropY);
    if (effectiveWidth <= 0 || effectiveHeight <= 0) {
        return result;
    }

    result.width = effectiveWidth;
    result.height = effectiveHeight;
    result.data.resize(static_cast<size_t>(result.width) * result.height);

    return result;
}

DepthImage maskDepth(const DepthImage& depth, const Image8& mask) {
    DepthImage result;
    result.width = depth.width;
    result.height = depth.height;
    result.data.resize(depth.data.size(), 0);

    const size_t count = std::min(depth.data.size(), mask.data.size());
    for (size_t i = 0; i < count; ++i) {
        // TODO: Keep the depth value for foreground pixels and set the background to 0.
    }

    return result;
}

DepthImage cropDepth(const DepthImage& depth, int cropX, int cropY, int cropW, int cropH) {
    DepthImage result;
    result.width = 0;
    result.height = 0;

    if (depth.width <= 0 || depth.height <= 0 || depth.data.empty()) {
        return result;
    }

    if (cropX >= depth.width || cropY >= depth.height) {
        return result;
    }

    const int effectiveWidth = std::min(cropW, depth.width - cropX);
    const int effectiveHeight = std::min(cropH, depth.height - cropY);
    if (effectiveWidth <= 0 || effectiveHeight <= 0) {
        return result;
    }

    result.width = effectiveWidth;
    result.height = effectiveHeight;
    result.data.resize(static_cast<size_t>(result.width) * result.height);

    // TODO: Copy the selected rectangular region into the output depth image.


    return result;
}
