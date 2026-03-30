#ifndef PROVIDED_FUNCTIONS_H
#define PROVIDED_FUNCTIONS_H

#include "student_functions.h"
#include <vector>

// ==============================
// Fonctions fournies
// ==============================

Image8 thresholdMask(const Image8& input);
std::vector<std::vector<PixelCoord>> extractConnectedComponents(const Image8& mask, int minComponentSize);
Image8 buildComponentPreview(const Image8& binaryMask, const std::vector<std::vector<PixelCoord>>& components);
Image8 buildMaskFromComponent(const Image8& referenceMask, const std::vector<PixelCoord>& component);
CropBox computeBoundingBox(const Image8& mask, int padding);
Image8 cropMask(const Image8& mask, const CropBox& cropBox);
void pasteMask(Image8& destination, const Image8& source, const CropBox& cropBox);
Image8 zhangSuenThinning(const Image8& mask);
std::vector<PixelCoord> skeletonNeighbors(const Image8& skeleton, const PixelCoord& point);
Image8 keepLargestComponent(const Image8& mask);
std::vector<PixelCoord> reconstructPath(const Image8& skeleton, const std::vector<int>& previous, const PixelCoord& end);
double pointLineDistance(Point2d point, Point2d start, Point2d end);
double circleRadiusFromThreePoints(Point2d a, Point2d b, Point2d c);
Image8 drawPixelPathPreview(const Image8& binaryMask, const Image8& skeletonAll, const std::vector<std::vector<PixelCoord>>& paths, const std::vector<CropBox>& cropBoxes);
Image8 drawPathPreview(const Image8& binaryMask, const Image8& skeletonAll, const std::vector<std::vector<Point2d>>& paths);
Image8 drawVisualization(const Image8& binaryMask, const Image8& skeletonAll, const std::vector<std::vector<Point2d>>& paths, const std::vector<CurvatureMetrics>& metricsList);

#endif // PROVIDED_FUNCTIONS_H
