#ifndef AST_H
#define AST_H

enum class NodeType {
    SYMBOL,
    UNION,
    CONCAT,
    STAR,
    PLUS,
    QUESTION
};

struct ASTNode {
    NodeType type;
    char value;
    ASTNode* left;
    ASTNode* right;
};

void printAST(const ASTNode* node, int depth = 0);



#endif