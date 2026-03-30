#include "io.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <gdiplus.h>
#ifdef _MSC_VER
#pragma comment(lib, "gdiplus.lib")
#endif
#endif

namespace {

bool hasTiffExtension(const std::string& path) {
    const size_t lastDot = path.find_last_of('.');
    if (lastDot == std::string::npos) {
        return false;
    }

    std::string extension = path.substr(lastDot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".tif" || extension == ".tiff";
}

bool hasBmpExtension(const std::string& path) {
    const size_t lastDot = path.find_last_of('.');
    if (lastDot == std::string::npos) {
        return false;
    }

    std::string extension = path.substr(lastDot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".bmp";
}

bool hasPngExtension(const std::string& path) {
    const size_t lastDot = path.find_last_of('.');
    if (lastDot == std::string::npos) {
        return false;
    }

    std::string extension = path.substr(lastDot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return extension == ".png";
}

bool readFileHeader(const std::string& path, std::vector<uint8_t>& header, size_t maxBytes) {
    header.clear();

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }

    header.resize(maxBytes, 0);
    file.read(reinterpret_cast<char*>(header.data()), static_cast<std::streamsize>(maxBytes));
    header.resize(static_cast<size_t>(file.gcount()));
    return !header.empty();
}

bool isBmpSignature(const std::vector<uint8_t>& header) {
    return header.size() >= 2 && header[0] == 'B' && header[1] == 'M';
}

bool isPngSignature(const std::vector<uint8_t>& header) {
    static const uint8_t pngSignature[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    return header.size() >= sizeof(pngSignature) &&
           std::equal(std::begin(pngSignature), std::end(pngSignature), header.begin());
}

uint16_t readU16(const std::vector<uint8_t>& bytes, size_t offset, bool littleEndian) {
    if (littleEndian) {
        return static_cast<uint16_t>(bytes[offset]) |
               (static_cast<uint16_t>(bytes[offset + 1]) << 8);
    }

    return (static_cast<uint16_t>(bytes[offset]) << 8) |
           static_cast<uint16_t>(bytes[offset + 1]);
}

uint32_t readU32(const std::vector<uint8_t>& bytes, size_t offset, bool littleEndian) {
    if (littleEndian) {
        return static_cast<uint32_t>(bytes[offset]) |
               (static_cast<uint32_t>(bytes[offset + 1]) << 8) |
               (static_cast<uint32_t>(bytes[offset + 2]) << 16) |
               (static_cast<uint32_t>(bytes[offset + 3]) << 24);
    }

    return (static_cast<uint32_t>(bytes[offset]) << 24) |
           (static_cast<uint32_t>(bytes[offset + 1]) << 16) |
           (static_cast<uint32_t>(bytes[offset + 2]) << 8) |
           static_cast<uint32_t>(bytes[offset + 3]);
}

float readF32(const std::vector<uint8_t>& bytes, size_t offset, bool littleEndian) {
    const uint32_t raw = readU32(bytes, offset, littleEndian);
    float value = 0.0f;
    std::memcpy(&value, &raw, sizeof(float));
    return value;
}

struct TiffEntry {
    uint16_t tag = 0;
    uint16_t type = 0;
    uint32_t count = 0;
    uint32_t valueOffset = 0;
    size_t entryOffset = 0;
};

size_t typeSize(uint16_t type) {
    switch (type) {
    case 3:
        return 2;
    case 4:
        return 4;
    case 11:
        return 4;
    default:
        return 0;
    }
}

bool readEntryValues(const std::vector<uint8_t>& bytes, bool littleEndian,
                     const TiffEntry& entry, std::vector<uint32_t>& values) {
    const size_t valueBytes = typeSize(entry.type);
    if (valueBytes == 0) {
        return false;
    }

    const size_t totalBytes = valueBytes * static_cast<size_t>(entry.count);
    size_t dataOffset = entry.entryOffset + 8;
    if (totalBytes > 4) {
        dataOffset = entry.valueOffset;
    }

    if (dataOffset + totalBytes > bytes.size()) {
        return false;
    }

    values.clear();
    values.reserve(entry.count);

    for (uint32_t i = 0; i < entry.count; ++i) {
        const size_t currentOffset = dataOffset + i * valueBytes;
        if (entry.type == 3) {
            values.push_back(readU16(bytes, currentOffset, littleEndian));
        } else if (entry.type == 4) {
            values.push_back(readU32(bytes, currentOffset, littleEndian));
        } else {
            return false;
        }
    }

    return true;
}

bool readScalarValue(const std::vector<uint8_t>& bytes, bool littleEndian,
                     const TiffEntry& entry, uint32_t& value) {
    std::vector<uint32_t> values;
    if (!readEntryValues(bytes, littleEndian, entry, values) || values.empty()) {
        return false;
    }
    value = values[0];
    return true;
}

DepthImage loadDepthBinary(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Erreur : impossible d'ouvrir le fichier " << path << std::endl;
        return {0, 0, {}};
    }

    int width, height;
    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));

    if (!file) {
        std::cerr << "Erreur : fichier trop court pour lire width/height" << std::endl;
        return {0, 0, {}};
    }

    const size_t size = static_cast<size_t>(width) * height;
    std::vector<uint16_t> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size * sizeof(uint16_t));

    if (!file) {
        std::cerr << "Erreur : fichier trop court pour lire les donnees" << std::endl;
        return {0, 0, {}};
    }

    return {width, height, std::move(data)};
}

Image8 loadImageBMPInternal(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Erreur : impossible d'ouvrir l'image " << path << std::endl;
        return {0, 0, {}};
    }

    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
    if (bytes.size() < 54) {
        std::cerr << "Erreur : fichier BMP trop court " << path << std::endl;
        return {0, 0, {}};
    }

    if (bytes[0] != 'B' || bytes[1] != 'M') {
        std::cerr << "Erreur : signature BMP invalide " << path << std::endl;
        return {0, 0, {}};
    }

    const uint32_t pixelOffset = readU32(bytes, 10, true);
    const int32_t width = static_cast<int32_t>(readU32(bytes, 18, true));
    const int32_t height = static_cast<int32_t>(readU32(bytes, 22, true));
    const uint16_t planes = readU16(bytes, 26, true);
    const uint16_t bitsPerPixel = readU16(bytes, 28, true);
    const uint32_t compression = readU32(bytes, 30, true);

    if (width <= 0 || height == 0 || planes != 1 || (bitsPerPixel != 8 && bitsPerPixel != 24) || compression != 0) {
        std::cerr << "Erreur : format BMP non supporte " << path << std::endl;
        return {0, 0, {}};
    }

    const int absHeight = (height < 0) ? -height : height;
    Image8 image;
    image.width = width;
    image.height = absHeight;
    image.data.resize(static_cast<size_t>(width) * absHeight, 0);

    if (bitsPerPixel == 24) {
        const uint32_t rowStride = static_cast<uint32_t>(width) * 3u;
        const uint32_t paddedRowStride = (rowStride + 3u) & ~3u;
        if (pixelOffset + static_cast<size_t>(paddedRowStride) * absHeight > bytes.size()) {
            std::cerr << "Erreur : donnees BMP hors limites " << path << std::endl;
            return {0, 0, {}};
        }

        for (int y = 0; y < absHeight; ++y) {
            const int srcY = (height > 0) ? (absHeight - 1 - y) : y;
            const size_t rowOffset = pixelOffset + static_cast<size_t>(srcY) * paddedRowStride;

            for (int x = 0; x < width; ++x) {
                const size_t pixelIndex = rowOffset + static_cast<size_t>(x) * 3;
                image.data[static_cast<size_t>(y) * width + x] = bytes[pixelIndex + 2]; // Red channel
            }
        }
    } else if (bitsPerPixel == 8) {
        // 8-bit BMP with palette
        const uint32_t paletteOffset = 54; // After 14-byte file header + 40-byte DIB header
        const uint32_t rowStride = static_cast<uint32_t>(width);
        const uint32_t paddedRowStride = (rowStride + 3u) & ~3u;
        if (pixelOffset + static_cast<size_t>(paddedRowStride) * absHeight > bytes.size()) {
            std::cerr << "Erreur : donnees BMP hors limites " << path << std::endl;
            return {0, 0, {}};
        }

        // Read palette (256 entries, 4 bytes each: B, G, R, 0)
        std::vector<uint8_t> palette(256 * 4);
        if (paletteOffset + palette.size() > bytes.size()) {
            std::cerr << "Erreur : palette BMP hors limites " << path << std::endl;
            return {0, 0, {}};
        }
        std::copy(bytes.begin() + paletteOffset, bytes.begin() + paletteOffset + palette.size(), palette.begin());

        for (int y = 0; y < absHeight; ++y) {
            const int srcY = (height > 0) ? (absHeight - 1 - y) : y;
            const size_t rowOffset = pixelOffset + static_cast<size_t>(srcY) * paddedRowStride;

            for (int x = 0; x < width; ++x) {
                const size_t pixelIndex = rowOffset + static_cast<size_t>(x);
                const uint8_t paletteIndex = bytes[pixelIndex];
                // Use red channel from palette
                image.data[static_cast<size_t>(y) * width + x] = palette[paletteIndex * 4 + 2];
            }
        }
    }

    return image;
}

#ifdef _WIN32
std::wstring utf8ToWide(const std::string& text) {
    if (text.empty()) {
        return std::wstring();
    }

    const int utf8Size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                             text.c_str(), -1, nullptr, 0);
    if (utf8Size > 0) {
        std::wstring wide(static_cast<size_t>(utf8Size), L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.c_str(), -1,
                            wide.data(), utf8Size);
        if (!wide.empty() && wide.back() == L'\0') {
            wide.pop_back();
        }
        return wide;
    }

    const int ansiSize = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, nullptr, 0);
    if (ansiSize > 0) {
        std::wstring wide(static_cast<size_t>(ansiSize), L'\0');
        MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, wide.data(), ansiSize);
        if (!wide.empty() && wide.back() == L'\0') {
            wide.pop_back();
        }
        return wide;
    }

    return std::wstring(text.begin(), text.end());
}

bool ensureGdiplusStarted() {
    static bool initialized = false;
    static ULONG_PTR token = 0;

    if (!initialized) {
        Gdiplus::GdiplusStartupInput startupInput;
        initialized = (Gdiplus::GdiplusStartup(&token, &startupInput, nullptr) == Gdiplus::Ok);
    }

    return initialized;
}

Image8 loadImagePNGWindows(const std::string& path) {
    if (!ensureGdiplusStarted()) {
        std::cerr << "Erreur : impossible d'initialiser GDI+ pour lire le PNG " << path << std::endl;
        return {0, 0, {}};
    }

    const std::wstring widePath = utf8ToWide(path);
    Gdiplus::Bitmap bitmap(widePath.c_str());
    const Gdiplus::Status status = bitmap.GetLastStatus();
    if (status != Gdiplus::Ok) {
        std::cerr << "Erreur : impossible d'ouvrir l'image PNG " << path
                  << " (GDI+ status=" << static_cast<int>(status) << ")" << std::endl;
        return {0, 0, {}};
    }

    const UINT width = bitmap.GetWidth();
    const UINT height = bitmap.GetHeight();
    if (width == 0 || height == 0) {
        std::cerr << "Erreur : dimensions PNG invalides " << path << std::endl;
        return {0, 0, {}};
    }

    Image8 image;
    image.width = static_cast<int>(width);
    image.height = static_cast<int>(height);
    image.data.resize(static_cast<size_t>(width) * height, 0);

    for (UINT y = 0; y < height; ++y) {
        for (UINT x = 0; x < width; ++x) {
            Gdiplus::Color color;
            if (bitmap.GetPixel(x, y, &color) != Gdiplus::Ok) {
                std::cerr << "Erreur : lecture PNG invalide " << path << std::endl;
                return {0, 0, {}};
            }

            image.data[static_cast<size_t>(y) * width + x] = color.GetRed();
        }
    }

    return image;
}
#endif

DepthImage loadDepthTiff(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Erreur : impossible d'ouvrir le fichier TIFF " << path << std::endl;
        return {0, 0, {}};
    }

    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
    if (bytes.size() < 8) {
        std::cerr << "Erreur : fichier TIFF trop court " << path << std::endl;
        return {0, 0, {}};
    }

    const bool littleEndian = (bytes[0] == 'I' && bytes[1] == 'I');
    const bool bigEndian = (bytes[0] == 'M' && bytes[1] == 'M');
    if (!littleEndian && !bigEndian) {
        std::cerr << "Erreur : ordre des octets TIFF invalide " << path << std::endl;
        return {0, 0, {}};
    }

    if (readU16(bytes, 2, littleEndian) != 42) {
        std::cerr << "Erreur : magic TIFF invalide " << path << std::endl;
        return {0, 0, {}};
    }

    const uint32_t ifdOffset = readU32(bytes, 4, littleEndian);
    if (ifdOffset + 2 > bytes.size()) {
        std::cerr << "Erreur : IFD TIFF invalide " << path << std::endl;
        return {0, 0, {}};
    }

    const uint16_t entryCount = readU16(bytes, ifdOffset, littleEndian);
    if (ifdOffset + 2 + static_cast<size_t>(entryCount) * 12 > bytes.size()) {
        std::cerr << "Erreur : entrees TIFF invalides " << path << std::endl;
        return {0, 0, {}};
    }

    std::vector<TiffEntry> entries;
    entries.reserve(entryCount);
    for (uint16_t i = 0; i < entryCount; ++i) {
        const size_t entryOffset = ifdOffset + 2 + static_cast<size_t>(i) * 12;
        entries.push_back({
            readU16(bytes, entryOffset, littleEndian),
            readU16(bytes, entryOffset + 2, littleEndian),
            readU32(bytes, entryOffset + 4, littleEndian),
            readU32(bytes, entryOffset + 8, littleEndian),
            entryOffset
        });
    }

    auto findEntry = [&](uint16_t tag) -> const TiffEntry* {
        for (const auto& entry : entries) {
            if (entry.tag == tag) {
                return &entry;
            }
        }
        return nullptr;
    };

    const TiffEntry* widthEntry = findEntry(0x0100);
    const TiffEntry* heightEntry = findEntry(0x0101);
    const TiffEntry* bitsEntry = findEntry(0x0102);
    const TiffEntry* compressionEntry = findEntry(0x0103);
    const TiffEntry* stripsEntry = findEntry(0x0111);
    const TiffEntry* samplesEntry = findEntry(0x0115);
    const TiffEntry* byteCountsEntry = findEntry(0x0117);
    const TiffEntry* sampleFormatEntry = findEntry(0x0153);

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t bitsPerSample = 0;
    uint32_t compression = 0;
    uint32_t samplesPerPixel = 1;
    uint32_t sampleFormat = 1;

    if (!widthEntry || !heightEntry || !bitsEntry || !compressionEntry || !stripsEntry || !byteCountsEntry ||
        !readScalarValue(bytes, littleEndian, *widthEntry, width) ||
        !readScalarValue(bytes, littleEndian, *heightEntry, height) ||
        !readScalarValue(bytes, littleEndian, *bitsEntry, bitsPerSample) ||
        !readScalarValue(bytes, littleEndian, *compressionEntry, compression)) {
        std::cerr << "Erreur : metadonnees TIFF manquantes ou invalides " << path << std::endl;
        return {0, 0, {}};
    }

    if (samplesEntry && !readScalarValue(bytes, littleEndian, *samplesEntry, samplesPerPixel)) {
        std::cerr << "Erreur : SamplesPerPixel TIFF invalide " << path << std::endl;
        return {0, 0, {}};
    }

    if (sampleFormatEntry && !readScalarValue(bytes, littleEndian, *sampleFormatEntry, sampleFormat)) {
        std::cerr << "Erreur : SampleFormat TIFF invalide " << path << std::endl;
        return {0, 0, {}};
    }

    if (compression != 1 || samplesPerPixel != 1) {
        std::cerr << "Erreur : TIFF non supporte (compression ou nombre de canaux) " << path << std::endl;
        return {0, 0, {}};
    }

    std::vector<uint32_t> stripOffsets;
    std::vector<uint32_t> stripByteCounts;
    if (!readEntryValues(bytes, littleEndian, *stripsEntry, stripOffsets) ||
        !readEntryValues(bytes, littleEndian, *byteCountsEntry, stripByteCounts) ||
        stripOffsets.size() != stripByteCounts.size()) {
        std::cerr << "Erreur : strips TIFF invalides " << path << std::endl;
        return {0, 0, {}};
    }

    DepthImage image;
    image.width = static_cast<int>(width);
    image.height = static_cast<int>(height);
    image.data.resize(static_cast<size_t>(width) * height, 0);

    size_t pixelIndex = 0;
    for (size_t strip = 0; strip < stripOffsets.size(); ++strip) {
        const size_t offset = stripOffsets[strip];
        const size_t byteCount = stripByteCounts[strip];

        if (offset + byteCount > bytes.size()) {
            std::cerr << "Erreur : donnees TIFF hors limites " << path << std::endl;
            return {0, 0, {}};
        }

        if (bitsPerSample == 32 && sampleFormat == 3) {
            if (byteCount % sizeof(float) != 0) {
                std::cerr << "Erreur : strip float32 TIFF invalide " << path << std::endl;
                return {0, 0, {}};
            }

            const size_t valueCount = byteCount / sizeof(float);
            if (pixelIndex + valueCount > image.data.size()) {
                std::cerr << "Erreur : taille TIFF incoherente " << path << std::endl;
                return {0, 0, {}};
            }

            for (size_t i = 0; i < valueCount; ++i) {
                const float value = readF32(bytes, offset + i * sizeof(float), littleEndian);
                if (!std::isfinite(value) || value <= 0.0f) {
                    image.data[pixelIndex++] = 0;
                } else if (value >= 65535.0f) {
                    image.data[pixelIndex++] = 65535;
                } else {
                    image.data[pixelIndex++] = static_cast<uint16_t>(std::lround(value));
                }
            }
        } else if (bitsPerSample == 16 && sampleFormat == 1) {
            if (byteCount % sizeof(uint16_t) != 0) {
                std::cerr << "Erreur : strip uint16 TIFF invalide " << path << std::endl;
                return {0, 0, {}};
            }

            const size_t valueCount = byteCount / sizeof(uint16_t);
            if (pixelIndex + valueCount > image.data.size()) {
                std::cerr << "Erreur : taille TIFF incoherente " << path << std::endl;
                return {0, 0, {}};
            }

            for (size_t i = 0; i < valueCount; ++i) {
                image.data[pixelIndex++] = readU16(bytes, offset + i * sizeof(uint16_t), littleEndian);
            }
        } else {
            std::cerr << "Erreur : format TIFF non supporte (bits/sample format) " << path << std::endl;
            return {0, 0, {}};
        }
    }

    if (pixelIndex != image.data.size()) {
        std::cerr << "Erreur : nombre de pixels TIFF incorrect " << path << std::endl;
        return {0, 0, {}};
    }

    return image;
}

} // namespace

DepthImage loadDepth(const std::string& path) {
    if (hasTiffExtension(path)) {
        return loadDepthTiff(path);
    }

    return loadDepthBinary(path);
}

Image8 loadImage(const std::string& path) {
    std::vector<uint8_t> header;
    if (readFileHeader(path, header, 8)) {
        if (isBmpSignature(header)) {
            return loadImageBMPInternal(path);
        }

#ifdef _WIN32
        if (isPngSignature(header)) {
            return loadImagePNGWindows(path);
        }
#endif
    }

    if (hasBmpExtension(path)) {
        return loadImageBMPInternal(path);
    }

#ifdef _WIN32
    if (hasPngExtension(path)) {
        std::cerr << "Avertissement : extension PNG detectee mais signature invalide, tentative via GDI+ "
                  << path << std::endl;
        return loadImagePNGWindows(path);
    }
#endif

    std::cerr << "Erreur : format d'image 8-bit non supporte " << path << std::endl;
    return {0, 0, {}};
}

void saveDepth(const DepthImage& img, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Erreur : impossible de creer le fichier " << path << std::endl;
        return;
    }

    file.write(reinterpret_cast<const char*>(&img.width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&img.height), sizeof(int));
    file.write(reinterpret_cast<const char*>(img.data.data()), img.data.size() * sizeof(uint16_t));
}

void saveImageBMP(const Image8& img, const std::string& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Erreur : impossible de creer le fichier " << path << std::endl;
        return;
    }

    const uint32_t rowStride = static_cast<uint32_t>(img.width) * 3u;
    const uint32_t paddedRowStride = (rowStride + 3u) & ~3u;
    const uint32_t pixelDataSize = paddedRowStride * static_cast<uint32_t>(img.height);
    const uint32_t fileSize = 14u + 40u + pixelDataSize;

    const uint8_t fileHeader[14] = {
        'B', 'M',
        static_cast<uint8_t>(fileSize & 0xFF),
        static_cast<uint8_t>((fileSize >> 8) & 0xFF),
        static_cast<uint8_t>((fileSize >> 16) & 0xFF),
        static_cast<uint8_t>((fileSize >> 24) & 0xFF),
        0, 0, 0, 0,
        54, 0, 0, 0
    };

    const uint8_t infoHeader[40] = {
        40, 0, 0, 0,
        static_cast<uint8_t>(img.width & 0xFF),
        static_cast<uint8_t>((img.width >> 8) & 0xFF),
        static_cast<uint8_t>((img.width >> 16) & 0xFF),
        static_cast<uint8_t>((img.width >> 24) & 0xFF),
        static_cast<uint8_t>(img.height & 0xFF),
        static_cast<uint8_t>((img.height >> 8) & 0xFF),
        static_cast<uint8_t>((img.height >> 16) & 0xFF),
        static_cast<uint8_t>((img.height >> 24) & 0xFF),
        1, 0,
        24, 0,
        0, 0, 0, 0,
        static_cast<uint8_t>(pixelDataSize & 0xFF),
        static_cast<uint8_t>((pixelDataSize >> 8) & 0xFF),
        static_cast<uint8_t>((pixelDataSize >> 16) & 0xFF),
        static_cast<uint8_t>((pixelDataSize >> 24) & 0xFF),
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    };

    file.write(reinterpret_cast<const char*>(fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<const char*>(infoHeader), sizeof(infoHeader));

    std::vector<uint8_t> row(paddedRowStride, 0);
    for (int y = img.height - 1; y >= 0; --y) {
        for (int x = 0; x < img.width; ++x) {
            const uint8_t value = img.data[static_cast<size_t>(y) * img.width + x];
            const size_t offset = static_cast<size_t>(x) * 3;
            row[offset + 0] = value;
            row[offset + 1] = value;
            row[offset + 2] = value;
        }
        file.write(reinterpret_cast<const char*>(row.data()), row.size());
    }
}

void saveImage(const Image8& img, const std::string& path) {
    if (!hasBmpExtension(path)) {
        std::cerr << "Avertissement : saveImage ecrit un fichier BMP, utilisez de preference une extension .bmp : "
                  << path << std::endl;
    }
    saveImageBMP(img, path);
}
