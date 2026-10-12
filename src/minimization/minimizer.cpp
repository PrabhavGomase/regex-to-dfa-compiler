#include "minimizer.h"
#include <queue>

DFA minimizeDFA(const DFA& dfa, const Limits& limits) {
    WorkBudget budget(limits);
    // Discover reachable states, including the implicit sink (-1).
    std::vector<int> states{dfa.start};
    std::map<int, int> ids{{dfa.start, 0}};
    for (size_t i = 0; i < states.size(); ++i) {
        budget.check();
        for (char c : dfa.alphabet) {
            int next = nextState(dfa, states[i], c);
            if (!ids.count(next)) {
                if (states.size() >= limits.dfaStates + 1)
                    throw std::runtime_error("Resource limit: minimization state budget exceeded");
                ids[next] = static_cast<int>(states.size());
                states.push_back(next);
            }
        }
    }
    std::vector<int> groups(states.size());
    for (size_t i = 0; i < states.size(); ++i) groups[i] = dfa.acceptStates.count(states[i]) ? 1 : 0;
    // Moore refinement: acceptance and successor block IDs form each signature.
    while (true) {
        budget.check();
        std::map<std::vector<int>, int> signatures;
        std::vector<int> refined(states.size());
        for (size_t i = 0; i < states.size(); ++i) {
            std::vector<int> key{groups[i]};
            for (char c : dfa.alphabet) key.push_back(groups[ids.at(nextState(dfa, states[i], c))]);
            auto inserted = signatures.emplace(key, static_cast<int>(signatures.size()));
            refined[i] = inserted.first->second;
        }
        if (refined == groups) break;
        groups.swap(refined);
    }
    DFA result;
    result.alphabet = dfa.alphabet;
    result.start = groups[0];
    int count = 0;
    for (int g : groups) count = std::max(count, g + 1);
    result.nfaSets.resize(count);
    for (size_t i = 0; i < states.size(); ++i) {
        int g = groups[i], s = states[i];
        // Here these sets record source DFA states, not NFA provenance.
        result.nfaSets[g].insert(s);
        if (dfa.acceptStates.count(s)) result.acceptStates.insert(g);
        for (char c : dfa.alphabet) result.transitions[g][c] = groups[ids.at(nextState(dfa, s, c))];
    }
    return result;
}
