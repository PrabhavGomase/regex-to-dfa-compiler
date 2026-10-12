#include "compiler/compiler.h"
#include "compiler/json.h"
#include "matcher/matcher.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    bool json = argc > 1;
    try {
        std::string mode = argc > 1 ? argv[1] : "", regex, input, candidate;
        if (mode == "--request") {
            std::getline(std::cin, mode);
            std::getline(std::cin, regex);
            std::getline(std::cin, input);
            std::getline(std::cin, candidate);
        } else if (argc > 2) {
            regex = argv[2];
            if (argc > 3) input = argv[3];
        }
        if (mode == "--benchmark") {
            int repetitions = argc > 2 ? std::stoi(argv[2]) : 20;
            auto results = runBenchmarks(repetitions);
            std::cout << "{\"cases\":[";
            for (size_t i = 0; i < results.size(); ++i) {
                if (i) std::cout << ',';
                std::cout << comparisonJSON(results[i]);
            }
            std::cout << "]}\n";
        } else if (mode == "--optimize" || mode == "optimize") {
            if (argc < 3 && argc > 1 && std::string(argv[1]) != "--request") throw std::runtime_error("Missing regex argument");
            std::cout << optimizationJSON(optimizeRegex(regex)) << '\n';
        } else if (mode == "--equivalent" || mode == "equivalent") {
            if (argc > 3) candidate = argv[3];
            else if (argc > 1 && std::string(argv[1]) != "--request") throw std::runtime_error("Usage: --equivalent REGEX CANDIDATE");
            std::cout << equivalenceJSON(verifyRewrite(regex, candidate)) << '\n';
        } else if (mode == "--compare" || mode == "compare") {
            if (argc < 3 && argc > 1 && std::string(argv[1]) != "--request") throw std::runtime_error("Missing regex argument");
            std::cout << comparisonJSON(compareAlgorithms(regex)) << '\n';
        } else if (mode == "--json" || mode == "compile") {
            if (argc < 3 && argc > 1 && std::string(argv[1]) != "--request") throw std::runtime_error("Missing regex argument");
            auto c = compileRegex(regex);
            std::cout << compilationJSON(regex, input, c) << '\n';
        } else if (mode.empty()) {
            std::cout << "Enter regular expression: ";
            std::getline(std::cin, regex);
            auto c = compileRegex(regex);
            std::cout << "\nTokens:\n";
            for (const auto& t : c.tokens) {
                std::cout << t.offset << ": " << tokenName(t.type);
                if (t.type == TokenType::LITERAL) std::cout << ' ' << jsonQuote(std::string(1, t.value));
                std::cout << '\n';
            }
            std::cout << "\nAST:\n"; printAST(c.ast.get());
            std::cout << "\nNFA:\n"; printNFA(c.nfa);
            std::cout << "\nDFA:\n"; printDFA(c.dfa);
            std::cout << "\nMinimized DFA (sets of original DFA states; -1 is sink):\n"; printDFA(c.minimized, "D");
            std::cout << "\nDirect DFA (sets of positions):\n"; printDFA(c.direct.dfa, "p");
            std::cout << "\nAlgorithms and minimization agree. Enter a test string (empty allowed): ";
            std::getline(std::cin, input);
            std::cout << (matchDFA(c.minimized, input) ? "ACCEPT\n" : "REJECT\n");
        } else throw std::runtime_error("Unknown mode. Use --json, --compare, --optimize, --equivalent, or --benchmark");
        return 0;
    } catch (const std::exception& e) {
        if (json) std::cout << "{\"error\":" << jsonQuote(e.what()) << "}\n";
        else std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
