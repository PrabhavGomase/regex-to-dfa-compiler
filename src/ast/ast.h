#ifndef AST_H
#define AST_H
#include <string>

enum class NodeType {
    EMPTY,
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
    ~ASTNode() { delete left; delete right; }
    ASTNode(const ASTNode&) = delete;
    ASTNode& operator=(const ASTNode&) = delete;
    ASTNode(NodeType type, char value = '\0', ASTNode* left = nullptr, ASTNode* right = nullptr)
        : type(type), value(value), left(left), right(right) {}
};

void printAST(const ASTNode* node, int depth = 0);
const char* nodeName(NodeType type);
std::string regexText(const ASTNode* node);
int nodeCount(const ASTNode* node);



#endif
