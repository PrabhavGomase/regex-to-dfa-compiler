#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

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
};

class Lexer {
public:
    explicit Lexer(const std::string& input);

    std::vector<Token> tokenize();
    private:
    std::string input;
    size_t position;

    Token readToken();
};

#endif
