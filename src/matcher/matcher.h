#ifndef REGEXLAB_MATCHER_H
#define REGEXLAB_MATCHER_H
#include <string>
#include "../dfa/dfa.h"
struct DFATrace {
    bool accepted;
    std::vector<int> states;
};
struct NFATrace {
    bool accepted;
    std::vector<std::set<int>> states;
};
DFATrace traceDFA(const DFA& dfa, const std::string& input);
NFATrace traceNFA(const NFA& nfa, const std::string& input);
bool matchDFA(const DFA& dfa, const std::string& input);
bool matchNFA(const NFA& nfa, const std::string& input);
#endif
