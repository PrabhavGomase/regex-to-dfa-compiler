#ifndef REGEXLAB_EQUIVALENCE_H
#define REGEXLAB_EQUIVALENCE_H
#include "../dfa/dfa.h"
#include <string>
struct EquivalenceResult {
    bool equivalent;
    std::string counterexample; // May be empty even when NOT equivalent.
    size_t visitedPairs;
};
EquivalenceResult equivalentDFA(const DFA& a, const DFA& b, const Limits& limits = Limits{});
#endif
