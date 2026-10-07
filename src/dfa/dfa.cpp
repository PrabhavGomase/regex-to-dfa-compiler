#include "dfa.h"

#include <iostream>
#include <queue>
#include <stack>

std::set<int> epsilonClosure(const NFA& nfa, const std::set<int>& states) {
    std::set<int> closure = states;
    std::stack<int> work;

    for (int s : states) {
        work.push(s);
    }

    while (!work.empty()) {
        int current = work.top();
        work.pop();

        for (const Transition& t : nfa.transitions) {
            if (t.from == current && t.symbol == EPSILON) {
                if (closure.insert(t.to).second) {
                    work.push(t.to);
                }
            }
        }
    }

    return closure;
}

std::set<int> moveOnSymbol(const NFA& nfa, const std::set<int>& states, char symbol) {
    std::set<int> result;

    for (const Transition& t : nfa.transitions) {
        if (t.symbol == symbol && states.count(t.from)) {
            result.insert(t.to);
        }
    }

    return result;
}

DFA buildDFA(const NFA& nfa) {
    DFA dfa;

    // Alphabet: every non-epsilon symbol used in the NFA
    std::set<char> symbols;
    for (const Transition& t : nfa.transitions) {
        if (t.symbol != EPSILON) {
            symbols.insert(t.symbol);
        }
    }
    dfa.alphabet.assign(symbols.begin(), symbols.end());

    std::map<std::set<int>, int> ids;
    std::queue<int> work;

    auto addState = [&](const std::set<int>& set) {
        int id = static_cast<int>(dfa.nfaSets.size());
        ids[set] = id;
        dfa.nfaSets.push_back(set);
        if (set.count(nfa.accept)) {
            dfa.acceptStates.insert(id);
        }
        work.push(id);
        return id;
    };

    dfa.start = addState(epsilonClosure(nfa, {nfa.start}));

    while (!work.empty()) {
        int current = work.front();
        work.pop();

        for (char symbol : dfa.alphabet) {
            std::set<int> target =
                epsilonClosure(nfa, moveOnSymbol(nfa, dfa.nfaSets[current], symbol));

            if (target.empty()) {
                continue;  // no dead state stored; a missing transition means reject
            }

            auto found = ids.find(target);
            int targetId = (found == ids.end()) ? addState(target) : found->second;

            dfa.transitions[current][symbol] = targetId;
        }
    }

    return dfa;
}

void printDFA(const DFA& dfa) {
    std::cout << "States: " << dfa.nfaSets.size() << '\n';
    std::cout << "Start: D" << dfa.start << '\n';

    std::cout << "Accept:";
    for (int s : dfa.acceptStates) {
        std::cout << " D" << s;
    }
    std::cout << '\n';

    std::cout << "State sets:\n";
    for (size_t i = 0; i < dfa.nfaSets.size(); i++) {
        std::cout << "  D" << i << " = {";
        bool first = true;
        for (int s : dfa.nfaSets[i]) {
            if (!first) std::cout << ", ";
            std::cout << "q" << s;
            first = false;
        }
        std::cout << "}\n";
    }

    std::cout << "Transitions:\n";
    for (const auto& from : dfa.transitions) {
        for (const auto& edge : from.second) {
            std::cout << "  D" << from.first << " --" << edge.first
                      << "--> D" << edge.second << '\n';
        }
    }
}