#include "student_functions.h"
#include <algorithm>

void normalizeDepth(DepthImage& depth) {
    if (depth.data.empty()) {
        return;
    }

    // TODO: Find the minimum and maximum depth values in the image.

    uint16_t min = NULL;
    uint16_t max = NULL;
    for (uint16_t value : depth.data) {
        if (min == NULL || value < min) min = value;
        if (min == NULL || value > max) max = value;
    }

    // Avoid division by zero if all values are the same.

    // TODO: Apply the linear normalization formula to map values to [0, 65535].


}

void invertDepth(DepthImage& depth) {
    for (auto& value : depth.data) {
        // TODO: Invert each depth value in place with: 65535 - value.
        // Ca le fait pas déjà ??
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
    for (size_t i = 0; i < mask.data.size(); i++) {

        // Set if you can check the value on the right, left, up or down
        int moduloHeight = i % mask.height;
        bool canGoHigher = moduloHeight != 0;
        bool canGoLower = moduloHeight != mask.height - 1;
        
        int moduloWidth = i % mask.width;
        bool canGoLeft = moduloWidth != 0;
        bool canGoRight = moduloWidth != mask.width - 1;

        // Check if the neighboors pass the comparaison 
        uint8_t value = mask.data[i];
        bool changeValue = false;
        if (canGoHigher) {
            if (mask.data[i - mask.width] == comparative) changeValue = true;
            if (canGoRight) {
                if (mask.data[i - mask.width + 1] == comparative) changeValue = true;
            }
            if (canGoLeft) {
                if (mask.data[i - mask.width - 1] == comparative) changeValue = true;
            }
        }
        if (canGoLower) {
            if (mask.data[i + mask.width] == 0) changeValue = true;
            if (canGoRight) {
                if (mask.data[i + mask.width + 1] == comparative) changeValue = true;
            }
            if (canGoLeft) {
                if (mask.data[i + mask.width - 1] == comparative) changeValue = true;
            }
        }
        if (canGoRight) {
            if (mask.data[i + 1] == comparative) changeValue = true;
        } 
        if (canGoLeft) {
            if (mask.data[i - 1] == comparative) changeValue = true;
        }

        if (changeValue) {
            result.data[i] = comparative;
        }
        else result.data[i] = value;
    }

    return result;
}

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
