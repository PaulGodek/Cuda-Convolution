#ifndef TP4_MINI_PYBIND_VECTOR_THRESHOLD_H
#define TP4_MINI_PYBIND_VECTOR_THRESHOLD_H

#include <cstdint>
#include <vector>

std::vector<uint8_t> threshold_vector_cpp(const uint16_t* values, int length, int threshold);

#endif // TP4_MINI_PYBIND_VECTOR_THRESHOLD_H
