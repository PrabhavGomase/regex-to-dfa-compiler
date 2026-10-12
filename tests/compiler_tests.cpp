#include "../src/compiler/compiler.h"
#include "../src/matcher/matcher.h"
#include "../src/analysis/optimizer.h"
#include <functional>
#include <iostream>
#include <regex>
namespace {
int checks = 0;
void require(bool ok, const std::string& message) {
    ++checks;
    if (!ok) throw std::runtime_error(message);
}
void fails(const std::function<void()>& fn, const std::string& fragment) {
    try { fn(); } catch (const std::runtime_error& e) {
        require(std::string(e.what()).find(fragment) != std::string::npos, "Wrong diagnostic: " + std::string(e.what()));
        return;
    }
    throw std::runtime_error("Expected error: " + fragment);
}
std::vector<std::string> words(const std::string& alphabet, int depth) {
    std::vector<std::string> all{""}, layer{""};
    for (int i = 0; i < depth; ++i) {
        std::vector<std::string> next;
        for (const auto& s : layer) for (char c : alphabet) next.push_back(s + c);
        all.insert(all.end(), next.begin(), next.end());
        layer.swap(next);
    }
    return all;
}
void agreement(const std::string& regex, const std::string& alphabet, int depth, bool oracle = true) {
    auto c = compileRegex(regex);
    require(equivalentDFA(c.dfa, c.minimized).equivalent, "Minimization disagreement: " + regex);
    require(equivalentDFA(c.dfa, c.direct.dfa).equivalent, "Direct disagreement: " + regex);
    require(equivalentDFA(c.direct.dfa, c.directMinimized).equivalent, "Direct minimization disagreement");
    std::regex reference;
    if (oracle) reference = std::regex(regex);
    for (const auto& s : words(alphabet, depth)) {
        bool expected = oracle ? std::regex_match(s, reference) : matchNFA(c.nfa, s);
        require(matchNFA(c.nfa, s) == expected, "NFA: " + regex + " / " + s);
        require(matchDFA(c.dfa, s) == expected, "DFA: " + regex + " / " + s);
        require(matchDFA(c.minimized, s) == expected, "Min: " + regex + " / " + s);
        require(matchDFA(c.direct.dfa, s) == expected, "Direct: " + regex + " / " + s);
        require(matchDFA(c.directMinimized, s) == expected, "Direct min: " + regex + " / " + s);
    }
}
}
int main() {
    try {
        for (const std::string r : {"", "()", "a", "ab", "a|b", "ab*", "a|bc", "a|b*", "a+b?",
             "(a|b)*abb", "a(b|c)*", "a?b+", "(a?)+", "(()|a)b?", "(a*|b*)*", "((a*)*)*", "a b", "#", "a()", "()a", "(a|())+"})
            agreement(r, r == "a b" ? "ab " : "abc", 4);
        for (const std::string r : {"a**", "a+?", "a?+", "a*+", "a??", "a++"}) agreement(r, "ab", 4, false);
        for (const std::string r : {"\\*", "\\+", "\\?", "\\(", "\\)", "\\|", "\\\\", "\\.", "\\[", "\\]", "\\{", "\\}", "\\^", "\\$"})
            agreement(r, r.substr(1) + "a", 2);
        agreement("\\ ", " a", 3, false);
        for (const std::string r : {"(", "a|", "|a", "a||b", "*a", "a)", "(a", "(a|)", "(?a)"})
            fails([&] { compileRegex(r); }, "offset");
        for (const std::string r : {".", "[a-z]", "a{2}", "^a", "a$", "\\d", "\\1", "\\n", "\\"})
            fails([&] { compileRegex(r); }, "offset");
        fails([] { compileRegex(std::string(1, '\0')); }, "ASCII");
        fails([] { compileRegex(std::string(1, static_cast<char>(200))); }, "ASCII");
        auto reference = compileRegex("(a|b)*abb");
        require(reference.nfa.stateCount == 14 && reference.dfa.nfaSets.size() == 5 && reference.minimized.nfaSets.size() == 4, "Reference state counts");
        for (const auto& s : {"abb", "aabb", "babb", "abababb"}) require(matchDFA(reference.minimized, s), "Known acceptance");
        for (const auto& s : {"", "ab", "abba", "abc"}) require(!matchDFA(reference.minimized, s), "Known rejection");
        // Hand-built partial DFA with an explicit dead state equivalent to missing edges and an unreachable acceptor.
        DFA partial{0, {1, 4}, {'a', 'b'}, std::vector<std::set<int>>(5), {{0, {{'a', 1}, {'b', 2}}}, {2, {{'a', 2}, {'b', 2}}}}};
        auto min = minimizeDFA(partial);
        require(min.nfaSets.size() == 3, "Dead-state merge and unreachable removal");
        require(equivalentDFA(partial, min).equivalent, "Partial DFA equivalence");
        DFA empty{0, {}, {'a'}, {{}}, {}};
        require(minimizeDFA(empty).nfaSets.size() == 1, "All rejecting states merge");
        DFA full{0, {0}, {'a'}, {{}}, {{0, {{'a', 0}}}}};
        require(minimizeDFA(full).nfaSets.size() == 1, "All accepting complete states merge");
        require(minimizeDFA(compileRegex("").dfa).nfaSets.size() == 1, "Empty alphabet");
        auto difference = verifyRewrite("a*", "a+");
        require(!difference.equivalent && difference.counterexample.empty(), "Empty witness");
        difference = verifyRewrite("a", "b");
        require(!difference.equivalent && difference.counterexample == "a", "Shortest deterministic witness over combined alphabet");
        difference = verifyRewrite("ab", "a");
        require(!difference.equivalent && matchDFA(compileRegex("ab").dfa, difference.counterexample) != matchDFA(compileRegex("a").dfa, difference.counterexample), "Witness validity");
        for (const std::string r : {"a|a", "((a*)*)*", "a* a*", "a*a*", "()a", "a()", "(a+)*", "(a?)*", "(a*)+", "(a*)?", "a++", "a??", "()*", "()+", "()?", "(ab|ab)c"}) {
            auto o = optimizeRegex(r);
            require(verifyRewrite(r, o.optimized).equivalent, "Optimizer equivalence: " + r);
            for (const auto& rewrite : o.rewrites) require(rewrite.afterNodes < rewrite.beforeNodes && rewrite.proof.equivalent, "Verified reduction");
        }
        require(optimizeRegex("a|a").optimized == "a", "Duplicate union optimization");
        require(optimizeRegex("ab").rewrites.empty(), "Already simple expression");
        require(!verifyRewrite("a|b", "ab").equivalent, "Invalid rewrite rejected");
        auto facts = compileRegex("ab").direct;
        require(facts.followpos.at(1) == std::set<int>{2} && facts.followpos.at(2) == std::set<int>{3}, "Known followpos");
        require(compileRegex("#").direct.endPosition == 2, "User # distinct from end marker");
        auto trace = traceDFA(reference.dfa, "abb");
        require(trace.accepted && trace.states.size() == 4 && trace.states.front() == reference.dfa.start, "DFA execution trace");
        require(traceNFA(reference.nfa, "abb").states.size() == 4, "NFA execution trace");
        fails([] { compileRegex(std::string(257, 'a')); }, "Resource limit");
        Limits limits;
        limits.dfaStates = 2;
        auto ast = parseRegex("abc");
        fails([&] { buildDFA(buildNFA(ast.get()), limits); }, "Resource limit");
        fails([&] { buildDirectDFA(ast.get(), limits); }, "Resource limit");
        limits = Limits{}; limits.nfaStates = 2;
        fails([&] { buildNFA(ast.get(), limits); }, "Resource limit");
        limits = Limits{}; limits.productStates = 1;
        fails([&] { equivalentDFA(reference.dfa, reference.direct.dfa, limits); }, "Resource limit");
        limits = Limits{}; limits.timeoutMs = 0;
        fails([&] { buildDFA(reference.nfa, limits); }, "timed out");
        fails([&] { minimizeDFA(reference.dfa, limits); }, "timed out");
        fails([&] { buildDirectDFA(reference.ast.get(), limits); }, "timed out");
        fails([&] { equivalentDFA(reference.dfa, reference.dfa, limits); }, "timed out");
        fails([&] { optimizeRegex("a|a", limits); }, "timed out");
        fails([&] { matchDFA(reference.dfa, std::string(4097, 'a')); }, "Resource limit");
        // Real exponential-family example: remembers the tenth symbol from the end.
        fails([] { compileRegex("(a|b)*a(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)"); }, "Resource limit");
        auto stress = parseRegex("(a|b)*a(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)");
        fails([&] { buildDirectDFA(stress.get()); }, "Resource limit");
        // Deterministic grammar-generated expressions supplement the hand-picked corpus.
        std::vector<std::string> generated{"a", "b", "()"};
        for (const auto& a : std::vector<std::string>{"a", "b", "()", "a?", "b*"})
            for (const auto& b : std::vector<std::string>{"a", "b", "()", "a+", "b?"}) {
                generated.push_back("(" + a + "|" + b + ")");
                generated.push_back("(" + a + ")(" + b + ")");
                generated.push_back("(" + a + "|" + b + ")*");
            }
        for (const auto& r : generated) agreement(r, "ab", 4);
        std::cout << "PASS: " << checks << " correctness assertions\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL after " << checks << " checks: " << e.what() << '\n';
        return 1;
    }
}
