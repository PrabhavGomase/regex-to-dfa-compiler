#include "nfa.h"

#include <iostream>
#include <stdexcept>

namespace {

struct Fragment {
    int start;
    int accept;
};

int newState(NFA& nfa) {
    return nfa.stateCount++;
}

void addTransition(NFA& nfa, int from, int to, char symbol) {
    nfa.transitions.push_back({from, to, symbol});
}

Fragment build(const ASTNode* node, NFA& nfa) {
    if (!node) {
        throw std::runtime_error("Cannot build NFA from empty AST");
    }

    switch (node->type) {
        case NodeType::SYMBOL: {
            int s = newState(nfa);
            int f = newState(nfa);
            addTransition(nfa, s, f, node->value);
            return {s, f};
        }

        case NodeType::CONCAT: {
            Fragment a = build(node->left, nfa);
            Fragment b = build(node->right, nfa);
            addTransition(nfa, a.accept, b.start, EPSILON);
            return {a.start, b.accept};
        }

        case NodeType::UNION: {
            Fragment a = build(node->left, nfa);
            Fragment b = build(node->right, nfa);
            int s = newState(nfa);
            int f = newState(nfa);
            addTransition(nfa, s, a.start, EPSILON);
            addTransition(nfa, s, b.start, EPSILON);
            addTransition(nfa, a.accept, f, EPSILON);
            addTransition(nfa, b.accept, f, EPSILON);
            return {s, f};
        }

        case NodeType::STAR: {
            Fragment a = build(node->left, nfa);
            int s = newState(nfa);
            int f = newState(nfa);
            addTransition(nfa, s, a.start, EPSILON);
            addTransition(nfa, s, f, EPSILON);
            addTransition(nfa, a.accept, a.start, EPSILON);
            addTransition(nfa, a.accept, f, EPSILON);
            return {s, f};
        }

        case NodeType::PLUS: {
            Fragment a = build(node->left, nfa);
            int s = newState(nfa);
            int f = newState(nfa);
            addTransition(nfa, s, a.start, EPSILON);
            addTransition(nfa, a.accept, a.start, EPSILON);
            addTransition(nfa, a.accept, f, EPSILON);
            return {s, f};
        }

        case NodeType::QUESTION: {
            Fragment a = build(node->left, nfa);
            int s = newState(nfa);
            int f = newState(nfa);
            addTransition(nfa, s, a.start, EPSILON);
            addTransition(nfa, s, f, EPSILON);
            addTransition(nfa, a.accept, f, EPSILON);
            return {s, f};
        }
    }

    throw std::runtime_error("Unknown AST node type");
}

} // namespace

NFA buildNFA(const ASTNode* root) {
    NFA nfa;
    nfa.stateCount = 0;

    Fragment result = build(root, nfa);
    nfa.start = result.start;
    nfa.accept = result.accept;

    return nfa;
}

void printNFA(const NFA& nfa) {
    std::cout << "States: " << nfa.stateCount << '\n';
    std::cout << "Start: q" << nfa.start << '\n';
    std::cout << "Accept: q" << nfa.accept << '\n';
    std::cout << "Transitions:\n";

    for (const Transition& t : nfa.transitions) {
        std::cout << "  q" << t.from << " --";
        if (t.symbol == EPSILON) {
            std::cout << "eps";
        } else {
            std::cout << t.symbol;
        }
        std::cout << "--> q" << t.to << '\n';
    }
}