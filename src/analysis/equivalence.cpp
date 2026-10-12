#include "equivalence.h"
#include <algorithm>
#include <queue>

EquivalenceResult equivalentDFA(const DFA& a, const DFA& b, const Limits& limits) {
    WorkBudget budget(limits);
    using Pair = std::pair<int, int>;
    std::set<char> alphabet(a.alphabet.begin(), a.alphabet.end());
    alphabet.insert(b.alphabet.begin(), b.alphabet.end());
    struct Predecessor { Pair parent; char symbol; };
    std::map<Pair, Predecessor> seen;
    std::queue<Pair> work;
    Pair start{a.start, b.start};
    if (!limits.productStates) throw std::runtime_error("Resource limit: product state budget exceeded");
    seen[start] = {start, '\0'};
    work.push(start);
    while (!work.empty()) {
        budget.check();
        Pair current = work.front(); work.pop();
        if ((a.acceptStates.count(current.first) != 0) != (b.acceptStates.count(current.second) != 0)) {
            std::string witness;
            for (Pair p = current; p != start; p = seen.at(p).parent) witness += seen.at(p).symbol;
            std::reverse(witness.begin(), witness.end());
            return {false, witness, seen.size()};
        }
        for (char c : alphabet) {
            Pair next{nextState(a, current.first, c), nextState(b, current.second, c)};
            if (seen.count(next)) continue;
            if (seen.size() >= limits.productStates)
                throw std::runtime_error("Resource limit: product state budget exceeded");
            seen[next] = {current, c};
            work.push(next);
        }
    }
    return {true, "", seen.size()};
}
