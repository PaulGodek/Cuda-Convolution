#include "student_functions.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <queue>
#include <string>
#include <vector>

namespace {

struct Point3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// Extracts 3D background samples from valid depth pixels outside the initial mask.
std::vector<Point3> collectBackgroundPoints(const DepthImage& depth, const Image8& initialMask) {
    std::vector<Point3> points;

    if (depth.width != initialMask.width || depth.height != initialMask.height ||
        depth.data.size() != initialMask.data.size()) {
        return points;
    }

    points.reserve(depth.data.size());

    for (int y = 0; y < depth.height; ++y) {
        const size_t rowOffset = static_cast<size_t>(y) * depth.width;
        for (int x = 0; x < depth.width; ++x) {
            const size_t index = rowOffset + static_cast<size_t>(x);
            const uint16_t z = depth.data[index];
            if (initialMask.data[index] != 0 || z == 0) {
                continue;
            }

            points.push_back(Point3{static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)});
        }
    }

    return points;
}

// Builds a plane passing through three non-collinear 3D points.
bool solvePlaneFromThreePoints(const Point3& p0, const Point3& p1, const Point3& p2, PlaneModel& plane) {
    // Build two direction vectors lying on the plane.
    const double ux = p1.x - p0.x;
    const double uy = p1.y - p0.y;
    const double uz = p1.z - p0.z;
    const double vx = p2.x - p0.x;
    const double vy = p2.y - p0.y;
    const double vz = p2.z - p0.z;

    // Their cross product gives a normal vector (a, b, c) for the plane.
    const double a = uy * vz - uz * vy;
    const double b = uz * vx - ux * vz;
    const double c = ux * vy - uy * vx;
    const double norm = std::sqrt(a * a + b * b + c * c);

    // Reject degenerate configurations where the three points are aligned.
    if (norm < 1e-9) {
        return false;
    }

    // Store the plane in implicit form: a*x + b*y + c*z + d = 0.
    plane.a = a;
    plane.b = b;
    plane.c = c;
    plane.d = -(a * p0.x + b * p0.y + c * p0.z);
    return true;
}

// Returns the Euclidean distance from a 3D point to a plane.
double pointPlaneDistance(const PlaneModel& plane, const Point3& point) {
    const double numerator = std::abs(plane.a * point.x + plane.b * point.y + plane.c * point.z + plane.d);
    const double denominator = std::sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
    if (denominator < 1e-9) {
        return 0.0;
    }
    return numerator / denominator;
}

// Solves a 3x3 linear system with Gaussian elimination and partial pivoting.
bool solve3x3(double matrix[3][4], double solution[3]) {
    for (int pivot = 0; pivot < 3; ++pivot) {
        int bestRow = pivot;
        for (int row = pivot + 1; row < 3; ++row) {
            if (std::abs(matrix[row][pivot]) > std::abs(matrix[bestRow][pivot])) {
                bestRow = row;
            }
        }

        if (std::abs(matrix[bestRow][pivot]) < 1e-9) {
            return false;
        }

        if (bestRow != pivot) {
            for (int col = pivot; col < 4; ++col) {
                std::swap(matrix[pivot][col], matrix[bestRow][col]);
            }
        }

        const double pivotValue = matrix[pivot][pivot];
        for (int col = pivot; col < 4; ++col) {
            matrix[pivot][col] /= pivotValue;
        }

        for (int row = 0; row < 3; ++row) {
            if (row == pivot) {
                continue;
            }

            const double factor = matrix[row][pivot];
            for (int col = pivot; col < 4; ++col) {
                matrix[row][col] -= factor * matrix[pivot][col];
            }
        }
    }

    solution[0] = matrix[0][3];
    solution[1] = matrix[1][3];
    solution[2] = matrix[2][3];
    return true;
}

// Refines a plane model by fitting z = alpha*x + beta*y + gamma on all inliers.
bool refinePlaneLeastSquares(const std::vector<Point3>& points, const std::vector<size_t>& inliers, PlaneModel& plane) {
    if (inliers.size() < 3) {
        return false;
    }

    double system[3][4] = {{0.0}};
    double solution[3] = {0.0, 0.0, 0.0};

    if (!solve3x3(system, solution)) {
        return false;
    }

    plane.a = solution[0];
    plane.b = solution[1];
    plane.c = -1.0;
    plane.d = solution[2];
    return true;
}

} // namespace

// Loads crop metadata from a simple key=value text file.
bool loadCropInfo(const std::string& path, CropInfo& info) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        const size_t separator = line.find('=');
        if (separator == std::string::npos) {
            continue;
        }

        const std::string key = line.substr(0, separator);
        const int value = std::stoi(line.substr(separator + 1));

        if (key == "originalWidth") info.originalWidth = value;
        else if (key == "originalHeight") info.originalHeight = value;
        else if (key == "cropX") info.cropX = value;
        else if (key == "cropY") info.cropY = value;
        else if (key == "cropW") info.cropW = value;
        else if (key == "cropH") info.cropH = value;
    }

    return info.originalWidth > 0 && info.originalHeight > 0 &&
           info.cropW > 0 && info.cropH > 0 &&
           info.cropX >= 0 && info.cropY >= 0;
}

// Extracts the cropped depth region described by the crop info.
DepthImage cropDepth(const DepthImage& depth, const CropInfo& info) {
    DepthImage cropped;
    cropped.width = 0;
    cropped.height = 0;

    if (depth.width <= 0 || depth.height <= 0 || depth.data.empty()) {
        return cropped;
    }

    if (info.cropX < 0 || info.cropY < 0 ||
        info.cropX + info.cropW > depth.width ||
        info.cropY + info.cropH > depth.height) {
        return cropped;
    }

    cropped.width = info.cropW;
    cropped.height = info.cropH;
    cropped.data.resize(static_cast<size_t>(cropped.width) * cropped.height, 0);

    for (int y = 0; y < cropped.height; ++y) {
        for (int x = 0; x < cropped.width; ++x) {
            cropped.data[static_cast<size_t>(y) * cropped.width + x] =
                depth.data[static_cast<size_t>(info.cropY + y) * depth.width + (info.cropX + x)];
        }
    }

    return cropped;
}

// Places a cropped mask back into its original full-image coordinates.
Image8 reconstructFullMask(const Image8& croppedMask, const CropInfo& info) {
    Image8 fullMask;
    fullMask.width = info.originalWidth;
    fullMask.height = info.originalHeight;

    if (fullMask.width <= 0 || fullMask.height <= 0 ||
        croppedMask.width != info.cropW || croppedMask.height != info.cropH) {
        fullMask.width = 0;
        fullMask.height = 0;
        return fullMask;
    }

    fullMask.data.assign(static_cast<size_t>(fullMask.width) * fullMask.height, 0);

    for (int y = 0; y < croppedMask.height; ++y) {
        for (int x = 0; x < croppedMask.width; ++x) {
            fullMask.data[static_cast<size_t>(info.cropY + y) * fullMask.width + (info.cropX + x)] =
                croppedMask.data[static_cast<size_t>(y) * croppedMask.width + x];
        }
    }

    return fullMask;
}

// Estimates the dominant background plane with RANSAC and least-squares refinement.
PlaneModel fitPlaneRansac(const DepthImage& depth, const Image8& initialMask,
                          int iterations, double inlierThreshold, bool& ok) {
    PlaneModel bestPlane;
    ok = false;

    const std::vector<Point3> points = collectBackgroundPoints(depth, initialMask);
    if (points.size() < 3) {
        return bestPlane;
    }

    // TODO: Implement RANSAC on the background points:
    // - random sample of 3 points
    // - candidate plane
    // - inlier counting with pointPlaneDistance
    // - keep the best model
    // - refine with refinePlaneLeastSquares

    return bestPlane;
}

// Converts each valid depth pixel into its positive signed distance to the plane.
DepthImage flattenDepthAgainstPlane(const DepthImage& depth, const PlaneModel& plane) {
    DepthImage flattened;
    flattened.width = depth.width;
    flattened.height = depth.height;
    flattened.data.resize(depth.data.size(), 0);

    const double denominator = std::sqrt(plane.a * plane.a + plane.b * plane.b + plane.c * plane.c);
    if (denominator < 1e-9) {
        return flattened;
    }

    // TODO: For each valid pixel (x, y, z), compute (a*x + b*y + c*z + d) /
    //       sqrt(a*a + b*b + c*c), then store max(0, distance).

    return flattened;
}

// Keeps only strong plane-distance responses as binary seed pixels.
Image8 thresholdStrongSeeds(const DepthImage& flattened, uint16_t seedThreshold) {
    Image8 seedMask;
    seedMask.width = flattened.width;
    seedMask.height = flattened.height;
    seedMask.data.resize(flattened.data.size(), 0);

    const size_t pixelCount = flattened.data.size();
    for (size_t i = 0; i < pixelCount; ++i) {
        seedMask.data[i] = static_cast<uint8_t>(-(flattened.data[i] >= seedThreshold));
    }

    return seedMask;
}

// Grows the seed mask over neighboring pixels that stay above the plane and close in depth.
Image8 regionGrowMask(const DepthImage& flattened, const Image8& seedMask,
                      uint16_t growDiff, uint16_t planeStopThreshold) {
    Image8 region = seedMask;

    if (flattened.width != seedMask.width || flattened.height != seedMask.height ||
        flattened.data.size() != seedMask.data.size()) {
        region.width = 0;
        region.height = 0;
        region.data.clear();
        return region;
    }

    std::queue<std::pair<int, int>> frontier;
    for (int y = 0; y < seedMask.height; ++y) {
        for (int x = 0; x < seedMask.width; ++x) {
            if (seedMask.data[static_cast<size_t>(y) * seedMask.width + x] != 0) {
                frontier.push({x, y});
            }
        }
    }

    // TODO: Implement an 8-connected region growing.
    // Accept a neighbor only if:
    // - it is not already in the region,
    // - its flattened value is strictly above planeStopThreshold,
    // - it is not much weaker than the current frontier value.

    return region;
}

// Counts pixels that are non-zero in both masks.
int countOverlap(const Image8& mask1, const Image8& mask2) {
    if (mask1.width != mask2.width || mask1.height != mask2.height || mask1.data.size() != mask2.data.size()) {
        return 0;
    }

    int overlap = 0;
    for (size_t i = 0; i < mask1.data.size(); ++i) {
        if (mask1.data[i] != 0 && mask2.data[i] != 0) {
            ++overlap;
        }
    }
    return overlap;
}

// Counts pixels that are non-zero in at least one of the masks.
int countUnion(const Image8& mask1, const Image8& mask2) {
    if (mask1.width != mask2.width || mask1.height != mask2.height || mask1.data.size() != mask2.data.size()) {
        return 0;
    }

    int unionCount = 0;
    for (size_t i = 0; i < mask1.data.size(); ++i) {
        if (mask1.data[i] != 0 || mask2.data[i] != 0) {
            ++unionCount;
        }
    }
    return unionCount;
}

// Computes overlap normalized by the full image size.
double computeSimpleScore(const Image8& mask1, const Image8& mask2) {
    if (mask1.width != mask2.width || mask1.height != mask2.height ||
        mask1.data.size() != mask2.data.size() || mask1.data.empty()) {
        return 0.0;
    }

    return static_cast<double>(countOverlap(mask1, mask2)) /
           static_cast<double>(mask1.data.size());
}

// Computes the intersection-over-union score of two binary masks.
double computeIoU(const Image8& mask1, const Image8& mask2) {
    if (mask1.width != mask2.width || mask1.height != mask2.height || mask1.data.size() != mask2.data.size()) {
        return 0.0;
    }

    const int unionCount = countUnion(mask1, mask2);
    if (unionCount == 0) {
        return 1.0;
    }

    return static_cast<double>(countOverlap(mask1, mask2)) /
           static_cast<double>(unionCount);
}

// Builds an 8-bit preview image from depth values for quick visualization.
Image8 makeDepthPreview(const DepthImage& depth, bool invert) {
    Image8 preview;
    preview.width = depth.width;
    preview.height = depth.height;
    preview.data.resize(depth.data.size(), invert ? 255 : 0);

    if (depth.data.empty()) {
        return preview;
    }

    uint16_t minValue = 65535;
    uint16_t maxValue = 0;
    bool hasValid = false;
    for (uint16_t value : depth.data) {
        if (value == 0) {
            continue;
        }
        hasValid = true;
        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
    }

    if (!hasValid) {
        return preview;
    }

    for (size_t i = 0; i < depth.data.size(); ++i) {
        const uint16_t value = depth.data[i];
        if (value == 0) {
            preview.data[i] = invert ? 255 : 0;
            continue;
        }

        if (maxValue == minValue) {
            preview.data[i] = 255;
            continue;
        }

        const uint32_t scaled = (static_cast<uint32_t>(value - minValue) * 255u) /
                                static_cast<uint32_t>(maxValue - minValue);
        preview.data[i] = invert ? static_cast<uint8_t>(255u - scaled) : static_cast<uint8_t>(scaled);
    }

    return preview;
}
