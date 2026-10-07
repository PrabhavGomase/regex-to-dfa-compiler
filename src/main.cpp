#include <iostream>
#include <string>
#include <vector>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "nfa/nfa.h"
#include "dfa/dfa.h"

static const char *tokenName(TokenType type)
{
    switch (type)
    {
    case TokenType::LITERAL:
        return "LITERAL";
    case TokenType::OR:
        return "OR";
    case TokenType::STAR:
        return "STAR";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::QUESTION:
        return "QUESTION";
    case TokenType::LPAREN:
        return "LPAREN";
    case TokenType::RPAREN:
        return "RPAREN";
    case TokenType::END:
        return "END";
    }
    return "UNKNOWN";
}

int main()
{
    std::string regex;

    std::cout << "Enter regular expression: ";
    std::getline(std::cin, regex);

    try
    {
        // Stage 1: Lexer
        Lexer lexer(regex);
        std::vector<Token> tokens = lexer.tokenize();

        std::cout << "\nTokens:\n";
        for (const Token &token : tokens)
        {
            std::cout << "Type: " << tokenName(token.type);

            if (token.type == TokenType::LITERAL)
            {
                std::cout << " | Value: " << token.value;
            }

            std::cout << '\n';
        }

        // Stage 2: Parser -> AST
        Parser parser(tokens);
        ASTNode *root = parser.parse();

        std::cout << "\nAST:\n";
        printAST(root);

        // Stage 3: Thompson construction -> epsilon-NFA
        NFA nfa = buildNFA(root);

        std::cout << "\nNFA:\n";
        printNFA(nfa);
        DFA dfa = buildDFA(nfa);

        std::cout << "\nDFA:\n";
        printDFA(dfa);
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}