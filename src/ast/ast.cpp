#include "ast.h"

#include <iostream>
#include <string>
#include <stdexcept>

void printAST(const ASTNode* node, int depth) {
    if (!node) return;

    std::cout << std::string(depth * 2, ' ');

    switch (node->type) {
        case NodeType::EMPTY: std::cout << "EMPTY"; break;
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

const char* nodeName(NodeType type) {
    switch (type) {
        case NodeType::EMPTY: return "EMPTY";
        case NodeType::SYMBOL: return "SYMBOL";
        case NodeType::UNION: return "UNION";
        case NodeType::CONCAT: return "CONCAT";
        case NodeType::STAR: return "STAR";
        case NodeType::PLUS: return "PLUS";
        case NodeType::QUESTION: return "QUESTION";
    }
    return "UNKNOWN";
}
std::string regexText(const ASTNode* node) {
    switch (node->type) {
        case NodeType::EMPTY: return "()";
        case NodeType::SYMBOL:
            return (std::string("|*+?()\\.[]{}^$").find(node->value) != std::string::npos ? "\\" : "") + std::string(1, node->value);
        case NodeType::UNION: return "(" + regexText(node->left) + "|" + regexText(node->right) + ")";
        case NodeType::CONCAT: return regexText(node->left) + regexText(node->right);
        case NodeType::STAR: return "(" + regexText(node->left) + ")*";
        case NodeType::PLUS: return "(" + regexText(node->left) + ")+";
        case NodeType::QUESTION: return "(" + regexText(node->left) + ")?";
    }
    throw std::runtime_error("Unknown AST node");
}
int nodeCount(const ASTNode* node) {
    return node ? 1 + nodeCount(node->left) + nodeCount(node->right) : 0;
}
