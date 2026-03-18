#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <thread>
#include <iostream>

namespace hft::core {

class ThreadUtils {
public:
    static void pin_to_core(int core_id) {
#ifdef _WIN32
        HANDLE thread = GetCurrentThread();
        DWORD_PTR mask = (static_cast<DWORD_PTR>(1) << core_id);
        if (SetThreadAffinityMask(thread, mask) == 0) {
            std::cerr << "Failed to set CPU affinity for core " << core_id << "\n";
        }
#else
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core_id, &cpuset);
        int rc = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
        if (rc != 0) {
            std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
        }
#endif
    }
};

} // namespace hft::core
