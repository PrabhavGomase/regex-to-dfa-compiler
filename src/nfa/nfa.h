#ifndef NFA_H
#define NFA_H

#include <vector>
#include "../ast/ast.h"

const char EPSILON = '\0';

struct Transition {
    int from;
    int to;
    char symbol;
};

struct NFA {
    int start;
    int accept;
    int stateCount;
    std::vector<Transition> transitions;
};

NFA buildNFA(const ASTNode* root);
void printNFA(const NFA& nfa);

#endif