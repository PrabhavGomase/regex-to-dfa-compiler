#ifndef REGEXLAB_DIRECT_H
#define REGEXLAB_DIRECT_H
#include "../dfa/dfa.h"
#include <string>
struct PositionFacts {
    std::string node;
    bool nullable;
    std::set<int> firstpos, lastpos;
};
struct DirectResult {
    DFA dfa;
    int endPosition;
    std::map<int, char> symbols;
    std::map<int, std::set<int>> followpos;
    std::vector<PositionFacts> facts;
};
DirectResult buildDirectDFA(const ASTNode* root, const Limits& limits = Limits{});
#endif
