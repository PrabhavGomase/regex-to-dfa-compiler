#include "lexer.h"

#include <stdexcept>

Lexer::Lexer(const std::string& input)
    : input(input), position(0) {}

Token Lexer::readToken() {
    while (position < input.length() &&
           input[position] == ' ') {
        position++;
    }

    if (position >= input.length()) {
        return {TokenType::END, '\0'};
    }

    char current = input[position++];

    switch (current) {
        case '|':
            return {TokenType::OR, current};

        case '*':
            return {TokenType::STAR, current};

        case '+':
            return {TokenType::PLUS, current};

        case '?':
            return {TokenType::QUESTION, current};

        case '(':
            return {TokenType::LPAREN, current};

        case ')':
            return {TokenType::RPAREN, current};

        case '\\':
            // Escaped character is treated as a literal.
            if (position >= input.length()) {
                throw std::runtime_error(
                    "Invalid escape sequence"
                );
            }

            return {
                TokenType::LITERAL,
                input[position++]
            };

        default:
            return {TokenType::LITERAL, current};
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        Token token = readToken();
        tokens.push_back(token);

        if (token.type == TokenType::END) {
            break;
        }
    }

    return tokens;
}