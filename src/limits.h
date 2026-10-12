#ifndef REGEXLAB_LIMITS_H
#define REGEXLAB_LIMITS_H
#include <chrono>
#include <cstddef>
#include <stdexcept>
struct Limits {
    std::size_t regexBytes = 256;
    std::size_t nfaStates = 2048;
    std::size_t dfaStates = 512;
    std::size_t productStates = 100000;
    int timeoutMs = 2000;
};
class WorkBudget {
    std::chrono::steady_clock::time_point deadline;
public:
    explicit WorkBudget(const Limits& limits)
        : deadline(std::chrono::steady_clock::now() + std::chrono::milliseconds(limits.timeoutMs)) {}
    void check() const {
        if (std::chrono::steady_clock::now() >= deadline)
            throw std::runtime_error("Resource limit: operation timed out");
    }
};
#endif
