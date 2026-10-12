#include "json.h"
#include "../matcher/matcher.h"
#include <iomanip>
#include <sstream>
namespace {
template<class Range> std::string numbers(const Range& range) {
    std::ostringstream out;
    out << '[';
    bool first = true;
    for (auto n : range) { if (!first) out << ','; first = false; out << n; }
    return out.str() + "]";
}
template<class Range> std::string sets(const Range& range) {
    std::string out = "[";
    for (const auto& set : range) { if (out.size() > 1) out += ','; out += numbers(set); }
    return out + "]";
}
std::string boolean(bool value) { return value ? "true" : "false"; }
std::string astJSON(const ASTNode* node) {
    if (!node) return "null";
    return "{\"type\":" + jsonQuote(nodeName(node->type)) + ",\"value\":" +
        (node->type == NodeType::SYMBOL ? jsonQuote(std::string(1, node->value)) : "null") +
        ",\"left\":" + astJSON(node->left) + ",\"right\":" + astJSON(node->right) + "}";
}
std::string dfaJSON(const DFA& dfa, const std::string& provenance) {
    std::ostringstream out;
    out << "{\"start\":" << dfa.start << ",\"accept\":" << numbers(dfa.acceptStates)
        << ",\"stateCount\":" << dfa.nfaSets.size() << ",\"sets\":" << sets(dfa.nfaSets)
        << ",\"provenance\":" << jsonQuote(provenance) << ",\"alphabet\":[";
    for (size_t i = 0; i < dfa.alphabet.size(); ++i) { if (i) out << ','; out << jsonQuote(std::string(1, dfa.alphabet[i])); }
    out << "],\"transitions\":[";
    bool first = true;
    for (const auto& row : dfa.transitions) for (const auto& edge : row.second) {
        if (!first) out << ',';
        first = false;
        out << "{\"from\":" << row.first << ",\"to\":" << edge.second << ",\"symbol\":" << jsonQuote(std::string(1, edge.first)) << '}';
    }
    return out.str() + "]}";
}
}
std::string jsonQuote(const std::string& text) {
    std::ostringstream out;
    out << '"';
    for (unsigned char c : text) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 32 || c > 126) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c) << std::dec;
        else out << c;
    }
    return out.str() + '"';
}
std::string equivalenceJSON(const EquivalenceResult& result) {
    return "{\"equivalent\":" + boolean(result.equivalent) + ",\"counterexample\":" +
        (result.equivalent ? "null" : jsonQuote(result.counterexample)) + ",\"visitedPairs\":" + std::to_string(result.visitedPairs) + "}";
}
std::string compilationJSON(const std::string& regex, const std::string& input, const Compilation& c) {
    std::ostringstream out;
    out << "{\"regex\":" << jsonQuote(regex) << ",\"tokens\":[";
    for (size_t i = 0; i < c.tokens.size(); ++i) {
        if (i) out << ',';
        auto t = c.tokens[i];
        out << "{\"type\":" << jsonQuote(tokenName(t.type)) << ",\"value\":"
            << (t.type == TokenType::END ? "null" : jsonQuote(std::string(1, t.value))) << ",\"offset\":" << t.offset << '}';
    }
    out << "],\"ast\":" << astJSON(c.ast.get()) << ",\"astNodes\":" << nodeCount(c.ast.get())
        << ",\"nfa\":{\"start\":" << c.nfa.start << ",\"accept\":[" << c.nfa.accept << "],\"stateCount\":" << c.nfa.stateCount << ",\"transitions\":[";
    for (size_t i = 0; i < c.nfa.transitions.size(); ++i) {
        if (i) out << ',';
        auto t = c.nfa.transitions[i];
        out << "{\"from\":" << t.from << ",\"to\":" << t.to << ",\"symbol\":" << (t.symbol == EPSILON ? "null" : jsonQuote(std::string(1, t.symbol))) << '}';
    }
    out << "]},\"dfa\":" << dfaJSON(c.dfa, "NFA states") << ",\"minimized\":" << dfaJSON(c.minimized, "original DFA states; -1 = sink")
        << ",\"direct\":" << dfaJSON(c.direct.dfa, "syntax-tree positions") << ",\"directMinimized\":" << dfaJSON(c.directMinimized, "direct DFA states; -1 = sink")
        << ",\"positions\":[";
    bool first = true;
    for (const auto& p : c.direct.followpos) {
        if (!first) out << ',';
        first = false;
        out << "{\"position\":" << p.first << ",\"symbol\":" << (p.first == c.direct.endPosition ? "null" : jsonQuote(std::string(1, c.direct.symbols.at(p.first))))
            << ",\"followpos\":" << numbers(p.second) << '}';
    }
    out << "],\"positionFacts\":[";
    for (size_t i = 0; i < c.direct.facts.size(); ++i) {
        if (i) out << ',';
        auto f = c.direct.facts[i];
        out << "{\"node\":" << jsonQuote(f.node) << ",\"nullable\":" << boolean(f.nullable) << ",\"firstpos\":" << numbers(f.firstpos) << ",\"lastpos\":" << numbers(f.lastpos) << '}';
    }
    auto d = traceDFA(c.dfa, input), m = traceDFA(c.minimized, input), direct = traceDFA(c.direct.dfa, input);
    auto n = traceNFA(c.nfa, input);
    if (d.accepted != n.accepted || m.accepted != d.accepted || direct.accepted != d.accepted)
        throw std::runtime_error("Internal error: matcher disagreement");
    out << "],\"agreement\":true,\"match\":{\"input\":" << jsonQuote(input) << ",\"accepted\":" << boolean(d.accepted)
        << ",\"dfaStates\":" << numbers(d.states) << ",\"minimizedStates\":" << numbers(m.states)
        << ",\"directStates\":" << numbers(direct.states) << ",\"nfaSets\":" << sets(n.states) << "}}";
    return out.str();
}
std::string comparisonJSON(const Comparison& c) {
    std::ostringstream out;
    out << std::setprecision(10) << "{\"regex\":" << jsonQuote(c.regex) << ",\"nfaStates\":" << c.nfaStates << ",\"nfaTransitions\":" << c.nfaTransitions
        << ",\"subset\":{\"states\":" << c.subsetStates << ",\"transitions\":" << c.subsetTransitions << ",\"minStates\":" << c.subsetMinStates << ",\"minTransitions\":" << c.subsetMinTransitions << "}"
        << ",\"direct\":{\"states\":" << c.directStates << ",\"transitions\":" << c.directTransitions << ",\"minStates\":" << c.directMinStates << ",\"minTransitions\":" << c.directMinTransitions << "}"
        << ",\"equivalent\":" << boolean(c.equivalent) << ",\"warmups\":" << c.warmups << ",\"repetitions\":" << c.repetitions << ",\"samples\":[";
    for (size_t i = 0; i < c.samples.size(); ++i) {
        if (i) out << ',';
        auto s = c.samples[i];
        out << "{\"method\":" << jsonQuote(s.method) << ",\"repetition\":" << s.repetition << ",\"constructionUs\":" << s.constructionUs << ",\"minimizationUs\":" << s.minimizationUs << '}';
    }
    return out.str() + "]}";
}
std::string optimizationJSON(const OptimizationResult& result) {
    std::string out = "{\"original\":" + jsonQuote(result.original) + ",\"optimized\":" + jsonQuote(result.optimized) + ",\"rewrites\":[";
    for (size_t i = 0; i < result.rewrites.size(); ++i) {
        if (i) out += ',';
        const auto& r = result.rewrites[i];
        out += "{\"rule\":" + jsonQuote(r.rule) + ",\"before\":" + jsonQuote(r.before) + ",\"after\":" + jsonQuote(r.after)
            + ",\"beforeNodes\":" + std::to_string(r.beforeNodes) + ",\"afterNodes\":" + std::to_string(r.afterNodes) + ",\"proof\":" + equivalenceJSON(r.proof) + "}";
    }
    return out + "]}";
}
