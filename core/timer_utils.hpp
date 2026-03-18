#pragma once

#include <cstdint>
#include <intrin.h> // For Windows/MSVC

namespace hft::core {

/**
 * @brief High-precision timer using CPU RDTSC instruction.
 */
class TimerUtils {
public:
    static inline uint64_t rdtsc() {
#ifdef _MSC_VER
        return __rdtsc();
#else
        unsigned int lo, hi;
        __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
        return ((uint64_t)hi << 32) | lo;
#endif
    }

    // Convert cycles to nanoseconds (approximate, needs calibration)
    // For now, we assume a constant cycle-to-ns ratio or just use cycles for relative comparison.
};

} // namespace hft::core
