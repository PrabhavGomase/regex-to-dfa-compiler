#ifndef REGEXLAB_OPTIMIZER_H
#define REGEXLAB_OPTIMIZER_H
#include "../compiler/compiler.h"
struct Rewrite {
    std::string rule, before, after;
    int beforeNodes, afterNodes;
    EquivalenceResult proof;
};
struct OptimizationResult {
    std::string original, optimized;
    std::vector<Rewrite> rewrites;
};
// Every accepted candidate must have fewer AST nodes and an equivalence proof.
OptimizationResult optimizeRegex(const std::string& regex, const Limits& limits = Limits{});
EquivalenceResult verifyRewrite(const std::string& before, const std::string& after, const Limits& limits = Limits{});
#endif
