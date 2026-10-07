#include "parser.h"


#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), position(0) {}

Token Parser::peek() const {
    if (position >= tokens.size()) {
        return {TokenType::END, '\0'};
    }

    return tokens[position];
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        position++;
        return true;
    }

    return false;
}

ASTNode* Parser::parse() {
    ASTNode* root = parseExpression();

    if (peek().type != TokenType::END) {
        throw std::runtime_error("Unexpected token after expression");
    }

    return root;
}

ASTNode* Parser::parseExpression() {
    ASTNode* left = parseTerm();

    while (match(TokenType::OR)) {
        ASTNode* right = parseTerm();

        ASTNode* node = new ASTNode;
        node->type = NodeType::UNION;
        node->value = '\0';
        node->left = left;
        node->right = right;

        left = node;
    }

    return left;
}

ASTNode* Parser::parseTerm() {
    ASTNode* left = parseFactor();

    while (true) {
        TokenType type = peek().type;

        if (type == TokenType::LITERAL ||
            type == TokenType::LPAREN) {

            ASTNode* right = parseFactor();

            ASTNode* node = new ASTNode;
            node->type = NodeType::CONCAT;
            node->value = '\0';
            node->left = left;
            node->right = right;

            left = node;
        } else {
            break;
        }
    }

    return left;
}

ASTNode* Parser::parseFactor() {
    ASTNode* node = parsePrimary();

    while (true) {
        if (match(TokenType::STAR)) {
            ASTNode* starNode = new ASTNode;
            starNode->type = NodeType::STAR;
            starNode->value = '\0';
            starNode->left = node;
            starNode->right = nullptr;

            node = starNode;
        }
        else if (match(TokenType::PLUS)) {
            ASTNode* plusNode = new ASTNode;
            plusNode->type = NodeType::PLUS;
            plusNode->value = '\0';
            plusNode->left = node;
            plusNode->right = nullptr;

            node = plusNode;
        }
        else if (match(TokenType::QUESTION)) {
            ASTNode* questionNode = new ASTNode;
            questionNode->type = NodeType::QUESTION;
            questionNode->value = '\0';
            questionNode->left = node;
            questionNode->right = nullptr;

            node = questionNode;
        }
        else {
            break;
        }
    }

    return node;
}

ASTNode* Parser::parsePrimary() {
    Token token = peek();

    if (match(TokenType::LITERAL)) {
        ASTNode* node = new ASTNode;
        node->type = NodeType::SYMBOL;
        node->value = token.value;
        node->left = nullptr;
        node->right = nullptr;

        return node;
    }

    if (match(TokenType::LPAREN)) {
        ASTNode* node = parseExpression();

        if (!match(TokenType::RPAREN)) {
            throw std::runtime_error("Missing closing parenthesis");
        }

        return node;
    }

    throw std::runtime_error("Expected literal or '('");
}