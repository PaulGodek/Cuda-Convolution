#include "../io.h"
#include "student_config.h"
#include "student_functions.h"
#include <cstdlib>
#include <fstream>
#include <limits>
#include <iostream>
#include <string>

using namespace std;

namespace {
bool parseThreshold(const string& text, uint16_t& threshold) {
    char* end = nullptr;
    const long value = strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0') {
        return false;
    }
    if (value < 0 || value > numeric_limits<uint16_t>::max()) {
        return false;
    }
    threshold = static_cast<uint16_t>(value);
    return true;
}
}

int main(int argc, char* argv[]) {
    if (argc > 3) {
        cout << "Usage: " << argv[0] << " [depth_image_path] [threshold]" << endl;
        cout << "You can also pass only a threshold to use the default image." << endl;
        return 1;
    }

    string depthPath = kStudentConfig.defaultDepthPath;
    uint16_t threshold = static_cast<uint16_t>(kStudentConfig.defaultThreshold);

    if (argc == 2) {
        const string arg1 = argv[1];
        if (!parseThreshold(arg1, threshold)) {
            depthPath = arg1;
        }
    } else if (argc == 3) {
        depthPath = argv[1];
        if (!parseThreshold(argv[2], threshold)) {
            cerr << "Threshold must be in [0, 65535]." << endl;
            return 1;
        }
    }

    cout << "Depth image: " << depthPath << endl;
    cout << "Threshold: " << threshold << endl;

    DepthImage depth = loadDepth(depthPath);
    if (depth.data.empty()) {
        cerr << "Failed to load depth image." << endl;
        return 1;
    }

    Image8 mask = thresholdDepth(depth, threshold);
    cout << "J open le mask" << endl;
    mask = openMask(mask);
    cout << "Je close le mask" << endl;
    mask = closeMask(mask);

    saveImage(mask, "mask_temporaire.png");
    return 0;

    Image8 croppedMask = cropMask(
        mask,
        kStudentConfig.cropX,
        kStudentConfig.cropY,
        kStudentConfig.cropW,
        kStudentConfig.cropH
    );

    DepthImage foreground = maskDepth(depth, mask);
    DepthImage cropped = cropDepth(
        foreground,
        kStudentConfig.cropX,
        kStudentConfig.cropY,
        kStudentConfig.cropW,
        kStudentConfig.cropH
    );
    

    if (cropped.data.empty() || croppedMask.data.empty()) {
        cerr << "No foreground detected after thresholding and morphology." << endl;
        return 1;
    }
    

    normalizeDepth(cropped);
    
    invertDepth(cropped);
    

    saveImage(croppedMask, "mask_final.png");
    
    saveDepth(cropped, "foreground_depth.bin");
    

    std::ofstream cropFile("mask_final_crop.txt");
    if (!cropFile) {
        std::cerr << "Failed to save crop metadata." << std::endl;
        return 1;
    }

    cropFile << "originalWidth=" << depth.width << "\n";
    cropFile << "originalHeight=" << depth.height << "\n";
    cropFile << "cropX=" << kStudentConfig.cropX << "\n";
    cropFile << "cropY=" << kStudentConfig.cropY << "\n";
    cropFile << "cropW=" << kStudentConfig.cropW << "\n";
    cropFile << "cropH=" << kStudentConfig.cropH << "\n";

    cout << "Pipeline complete." << endl;
    cout << "Saved: mask_final.png" << endl;
    cout << "Saved: foreground_depth.bin" << endl;
    cout << "Saved: mask_final_crop.txt" << endl;

    return 0;
}
