#include "ast.h"

#include <iostream>
#include <string>

void printAST(const ASTNode* node, int depth) {
    if (!node) return;

    std::cout << std::string(depth * 2, ' ');

    switch (node->type) {
        case NodeType::SYMBOL:   std::cout << "SYMBOL " << node->value; break;
        case NodeType::UNION:    std::cout << "UNION";    break;
        case NodeType::CONCAT:   std::cout << "CONCAT";   break;
        case NodeType::STAR:     std::cout << "STAR";     break;
        case NodeType::PLUS:     std::cout << "PLUS";     break;
        case NodeType::QUESTION: std::cout << "QUESTION"; break;
    }

    std::cout << '\n';

    printAST(node->left, depth + 1);
    printAST(node->right, depth + 1);
}