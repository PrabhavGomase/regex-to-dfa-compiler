#include "compiler.h"
std::unique_ptr<ASTNode> parseRegex(const std::string& regex, const Limits& limits) {
    auto tokens = Lexer(regex, limits).tokenize();
    return std::unique_ptr<ASTNode>(Parser(tokens).parse());
}
Compilation compileRegex(const std::string& regex, const Limits& limits) {
    Compilation c;
    c.tokens = Lexer(regex, limits).tokenize();
    c.ast.reset(Parser(c.tokens).parse());
    c.nfa = buildNFA(c.ast.get(), limits);
    c.dfa = buildDFA(c.nfa, limits);
    c.minimized = minimizeDFA(c.dfa, limits);
    c.direct = buildDirectDFA(c.ast.get(), limits);
    c.directMinimized = minimizeDFA(c.direct.dfa, limits);
    if (!equivalentDFA(c.dfa, c.direct.dfa, limits).equivalent ||
        !equivalentDFA(c.dfa, c.minimized, limits).equivalent ||
        !equivalentDFA(c.direct.dfa, c.directMinimized, limits).equivalent)
        throw std::runtime_error("Internal error: construction/minimization disagreement");
    return c;
}
