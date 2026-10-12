#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "../limits.h"

enum class TokenType {
    LITERAL,
    OR,          // |
    STAR,        // *
    PLUS,        // +
    QUESTION,    // ?
    LPAREN,      // (
    RPAREN,      // )
    END
};

struct Token {
    TokenType type;
    char value;
    size_t offset = 0;
};
const char* tokenName(TokenType type);

class Lexer {
public:
    explicit Lexer(const std::string& input, const Limits& limits = Limits{});

    std::vector<Token> tokenize();
    private:
    std::string input;
    size_t position;

    Token readToken();
};

#endif
