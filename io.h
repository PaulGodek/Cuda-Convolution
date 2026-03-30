#ifndef IO_H
#define IO_H

#include <cstdint>
#include <string>
#include <vector>

struct DepthImage {
    int width, height;
    std::vector<uint16_t> data;
};

struct Image8 {
    int width, height;
    std::vector<uint8_t> data;
};

// Supported depth formats:
// - custom binary: int width, int height, then width*height uint16_t pixels
// - uncompressed single-channel TIFF depth images
DepthImage loadDepth(const std::string& path);

// Supported 8-bit image formats:
// - BMP (8-bit and 24-bit)
// - PNG on Windows
Image8 loadImage(const std::string& path);

void saveDepth(const DepthImage& img, const std::string& path);
void saveImage(const Image8& img, const std::string& path);
void saveImageBMP(const Image8& img, const std::string& path);

#endif // IO_H
