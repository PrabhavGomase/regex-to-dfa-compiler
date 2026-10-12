#include "benchmark.h"
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
namespace {
double nowUs() {
#ifdef _WIN32
    // Older MinGW steady_clock has millisecond resolution. QPC resolves short builds.
    static const double frequency = [] {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        return static_cast<double>(f.QuadPart);
    }();
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<double>(counter.QuadPart) * 1000000.0 / frequency;
#else
    return std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now().time_since_epoch()).count();
#endif
}
}
size_t transitionCount(const DFA& dfa) {
    size_t count = 0;
    for (const auto& row : dfa.transitions) count += row.second.size();
    return count;
}
Comparison compareAlgorithms(const std::string& regex, int repetitions, const Limits& limits) {
    if (repetitions < 1 || repetitions > 100) throw std::runtime_error("Repetitions must be between 1 and 100");
    auto c = compileRegex(regex, limits);
    Comparison result{regex, static_cast<size_t>(c.nfa.stateCount), c.nfa.transitions.size(),
        c.dfa.nfaSets.size(), transitionCount(c.dfa), c.minimized.nfaSets.size(), transitionCount(c.minimized),
        c.direct.dfa.nfaSets.size(), transitionCount(c.direct.dfa), c.directMinimized.nfaSets.size(), transitionCount(c.directMinimized),
        equivalentDFA(c.dfa, c.direct.dfa, limits).equivalent, 3, repetitions, {}};
    auto measure = [&](bool direct, int repetition) {
        auto start = nowUs();
        DFA dfa;
        if (direct) dfa = buildDirectDFA(c.ast.get(), limits).dfa;
        else dfa = buildDFA(buildNFA(c.ast.get(), limits), limits);
        auto constructed = nowUs();
        auto minimized = minimizeDFA(dfa, limits);
        auto end = nowUs();
        // Inspect the result outside timing so the optimizer cannot discard work.
        if (minimized.nfaSets.empty()) throw std::runtime_error("Empty benchmark result");
        if (repetition >= 0) result.samples.push_back({direct ? "direct" : "thompson_subset", repetition,
            constructed - start, end - constructed});
    };
    for (int i = -result.warmups; i < repetitions; ++i) {
        // Alternate order to reduce systematic cache/ordering bias.
        bool first = (i % 2) != 0;
        measure(first, i);
        measure(!first, i);
    }
    return result;
}
std::vector<Comparison> runBenchmarks(int repetitions) {
    std::vector<Comparison> results;
    for (const std::string regex : {"a", "ab", "a|b|c|d", "(a|b)*abb", "((a*)*)*", "a?b+c*", "(a|b|c)*abc", "(a|b)*a(a|b)(a|b)(a|b)", "(a|b)*a(a|b)(a|b)(a|b)(a|b)(a|b)", "(a|b)*a(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)"})
        results.push_back(compareAlgorithms(regex, repetitions));
    return results;
}
