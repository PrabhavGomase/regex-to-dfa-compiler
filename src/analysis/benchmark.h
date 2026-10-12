#ifndef REGEXLAB_BENCHMARK_H
#define REGEXLAB_BENCHMARK_H
#include "../compiler/compiler.h"
struct Measurement {
    std::string method;
    int repetition;
    double constructionUs, minimizationUs;
};
struct Comparison {
    std::string regex;
    size_t nfaStates, nfaTransitions;
    size_t subsetStates, subsetTransitions, subsetMinStates, subsetMinTransitions;
    size_t directStates, directTransitions, directMinStates, directMinTransitions;
    bool equivalent;
    int warmups, repetitions;
    std::vector<Measurement> samples;
};
size_t transitionCount(const DFA& dfa);
Comparison compareAlgorithms(const std::string& regex, int repetitions = 10, const Limits& limits = Limits{});
std::vector<Comparison> runBenchmarks(int repetitions = 20);
#endif
