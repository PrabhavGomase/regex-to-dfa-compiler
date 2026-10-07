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
    std::vector<std::set<int>> nfaSets;              // NFA states behind each DFA state
    std::map<int, std::map<char, int>> transitions;  // transitions[from][symbol] = to
};

std::set<int> epsilonClosure(const NFA& nfa, const std::set<int>& states);
std::set<int> moveOnSymbol(const NFA& nfa, const std::set<int>& states, char symbol);

DFA buildDFA(const NFA& nfa);
void printDFA(const DFA& dfa);

#endif