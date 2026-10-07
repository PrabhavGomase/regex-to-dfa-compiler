#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "../lexer/lexer.h"
#include "../ast/ast.h"

class Parser {
private:
    std::vector<Token> tokens;
    size_t position;

    ASTNode* parseExpression();
    ASTNode* parseTerm();
    ASTNode* parseFactor();
    ASTNode* parsePrimary();

    bool match(TokenType type);
    Token peek() const;

public:
    explicit Parser(const std::vector<Token>& tokens);

    ASTNode* parse();
};

#endif