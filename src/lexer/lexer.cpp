#include "lexer.h"
#include <stdexcept>

const char* tokenName(TokenType type) {
    switch (type) {
        case TokenType::LITERAL: return "LITERAL";
        case TokenType::OR: return "OR";
        case TokenType::STAR: return "STAR";
        case TokenType::PLUS: return "PLUS";
        case TokenType::QUESTION: return "QUESTION";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::END: return "END";
    }
    return "UNKNOWN";
}

Lexer::Lexer(const std::string& input, const Limits& limits) : input(input), position(0) {
    if (input.size() > limits.regexBytes)
        throw std::runtime_error("Resource limit: regex exceeds " + std::to_string(limits.regexBytes) + " bytes");
}
Token Lexer::readToken() {
    size_t offset = position;
    if (position == input.size()) return {TokenType::END, '\0', offset};
    char c = input[position++];
    if (c < 32 || static_cast<unsigned char>(c) > 126)
        throw std::runtime_error("Only printable ASCII is supported at offset " + std::to_string(offset));
    switch (c) {
        case '|': return {TokenType::OR, c, offset};
        case '*': return {TokenType::STAR, c, offset};
        case '+': return {TokenType::PLUS, c, offset};
        case '?': return {TokenType::QUESTION, c, offset};
        case '(': return {TokenType::LPAREN, c, offset};
        case ')': return {TokenType::RPAREN, c, offset};
        case '\\': {
            if (position == input.size()) throw std::runtime_error("Trailing escape at offset " + std::to_string(offset));
            char escaped = input[position++];
            if (escaped < 32 || static_cast<unsigned char>(escaped) > 126 ||
                (escaped >= '0' && escaped <= '9') || (escaped >= 'A' && escaped <= 'Z') ||
                (escaped >= 'a' && escaped <= 'z'))
                throw std::runtime_error("Unsupported escape at offset " + std::to_string(offset) + "; escape punctuation only");
            return {TokenType::LITERAL, escaped, offset};
        }
        default:
            if (std::string(".[]{}^$").find(c) != std::string::npos)
                throw std::runtime_error("Unsupported metacharacter at offset " + std::to_string(offset) + "; escape it for a literal");
            return {TokenType::LITERAL, c, offset};
    }
}
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    do { tokens.push_back(readToken()); } while (tokens.back().type != TokenType::END);
    return tokens;
}
