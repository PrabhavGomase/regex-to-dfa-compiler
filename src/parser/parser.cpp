#include "parser.h"
#include <memory>
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {}
Token Parser::peek() const {
    return position < tokens.size() ? tokens[position] : Token{TokenType::END, '\0', position};
}
bool Parser::match(TokenType type) {
    if (peek().type != type) return false;
    ++position;
    return true;
}
ASTNode* Parser::parse() {
    if (match(TokenType::END)) return new ASTNode(NodeType::EMPTY);
    std::unique_ptr<ASTNode> root(parseExpression());
    if (peek().type != TokenType::END)
        throw std::runtime_error("Unexpected token at offset " + std::to_string(peek().offset));
    return root.release();
}
ASTNode* Parser::parseExpression() {
    std::unique_ptr<ASTNode> left(parseTerm());
    while (match(TokenType::OR)) {
        std::unique_ptr<ASTNode> right(parseTerm());
        left.reset(new ASTNode(NodeType::UNION, '\0', left.release(), right.release()));
    }
    return left.release();
}
ASTNode* Parser::parseTerm() {
    std::unique_ptr<ASTNode> left(parseFactor());
    while (peek().type == TokenType::LITERAL || peek().type == TokenType::LPAREN) {
        std::unique_ptr<ASTNode> right(parseFactor());
        left.reset(new ASTNode(NodeType::CONCAT, '\0', left.release(), right.release()));
    }
    return left.release();
}
ASTNode* Parser::parseFactor() {
    std::unique_ptr<ASTNode> node(parsePrimary());
    while (true) {
        NodeType type;
        if (match(TokenType::STAR)) type = NodeType::STAR;
        else if (match(TokenType::PLUS)) type = NodeType::PLUS;
        else if (match(TokenType::QUESTION)) type = NodeType::QUESTION;
        else break;
        node.reset(new ASTNode(type, '\0', node.release()));
    }
    return node.release();
}
ASTNode* Parser::parsePrimary() {
    Token token = peek();
    if (match(TokenType::LITERAL)) return new ASTNode(NodeType::SYMBOL, token.value);
    if (match(TokenType::LPAREN)) {
        if (match(TokenType::RPAREN)) return new ASTNode(NodeType::EMPTY);
        std::unique_ptr<ASTNode> node(parseExpression());
        if (!match(TokenType::RPAREN))
            throw std::runtime_error("Missing ')' at offset " + std::to_string(peek().offset));
        return node.release();
    }
    throw std::runtime_error("Expected literal or '(' at offset " + std::to_string(token.offset));
}
