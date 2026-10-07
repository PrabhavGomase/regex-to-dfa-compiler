#include <iostream>
#include <vector>
#include "parser/parser.h"
#include "ast/ast.h"
#include "lexer/lexer.h"

int main()
{
    std::string regex;

    std::cout << "Enter regular expression: ";
    std::getline(std::cin, regex);

    try
    {
        Lexer lexer(regex);
        std::vector<Token> tokens = lexer.tokenize();

        std::cout << "\nTokens:\n";

        for (const Token &token : tokens)
        {
            std::cout << "Type: ";

            switch (token.type)
            {
            case TokenType::LITERAL:
                std::cout << "LITERAL";
                break;

            case TokenType::OR:
                std::cout << "OR";
                break;

            case TokenType::STAR:
                std::cout << "STAR";
                break;

            case TokenType::PLUS:
                std::cout << "PLUS";
                break;

            case TokenType::QUESTION:
                std::cout << "QUESTION";
                break;

            case TokenType::LPAREN:
                std::cout << "LPAREN";
                break;

            case TokenType::RPAREN:
                std::cout << "RPAREN";
                break;

            case TokenType::END:
                std::cout << "END";
                break;
            }

            if (token.type == TokenType::LITERAL)
            {
                std::cout << " | Value: " << token.value;
            }

            std::cout << '\n';
        }
        Parser parser(tokens);
        ASTNode *root = parser.parse();

        std::cout << "\nAST:\n";
        printAST(root);
    }
    catch (const std::exception &error)
    {
        std::cerr << "Lexer Error: "
                  << error.what() << '\n';

        return 1;
    }

    return 0;
}