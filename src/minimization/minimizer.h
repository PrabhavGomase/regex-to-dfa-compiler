#ifndef REGEXLAB_MINIMIZER_H
#define REGEXLAB_MINIMIZER_H
#include "../dfa/dfa.h"
// Returns a complete, reachable DFA; absent edges share one rejecting sink.
DFA minimizeDFA(const DFA& dfa, const Limits& limits = Limits{});
#endif
