#ifndef DFA_H
#define DFA_H

#include <map>
#include <set>
#include <vector>

#include "../nfa/nfa.h"

struct DFA {
    int start;
    std::set<int> acceptStates;
    std::vector<char> alphabet;
    std::vector<std::set<int>> nfaSets;              // Backing NFA states, positions, or source DFA states (minimized)
    std::map<int, std::map<char, int>> transitions;  // transitions[from][symbol] = to
};

std::set<int> epsilonClosure(const NFA& nfa, const std::set<int>& states);
std::set<int> moveOnSymbol(const NFA& nfa, const std::set<int>& states, char symbol);

DFA buildDFA(const NFA& nfa, const Limits& limits = Limits{});
int nextState(const DFA& dfa, int state, char symbol);
void printDFA(const DFA& dfa, const char* setPrefix = "q");

#endif
