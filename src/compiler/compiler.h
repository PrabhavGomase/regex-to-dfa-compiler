#ifndef REGEXLAB_COMPILER_H
#define REGEXLAB_COMPILER_H
#include <memory>
#include "../parser/parser.h"
#include "../direct/direct.h"
#include "../minimization/minimizer.h"
#include "../analysis/equivalence.h"
struct Compilation {
    std::vector<Token> tokens;
    std::unique_ptr<ASTNode> ast;
    NFA nfa;
    DFA dfa, minimized;
    DirectResult direct;
    DFA directMinimized;
};
Compilation compileRegex(const std::string& regex, const Limits& limits = Limits{});
std::unique_ptr<ASTNode> parseRegex(const std::string& regex, const Limits& limits = Limits{});
#endif
