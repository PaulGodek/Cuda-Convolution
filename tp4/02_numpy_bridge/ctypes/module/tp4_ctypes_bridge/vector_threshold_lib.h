#ifndef TP4_CTYPES_BRIDGE_VECTOR_THRESHOLD_LIB_H
#define TP4_CTYPES_BRIDGE_VECTOR_THRESHOLD_LIB_H

#include <cstdint>

#if defined(_WIN32)
#define TP4_BRIDGE_EXPORT extern "C" __declspec(dllexport)
#else
#define TP4_BRIDGE_EXPORT extern "C"
#endif

TP4_BRIDGE_EXPORT int tp4ThresholdVectorBuffer(
    const uint16_t* values,
    int length,
    int threshold,
    uint8_t* outputData);

#endif // TP4_CTYPES_BRIDGE_VECTOR_THRESHOLD_LIB_H
