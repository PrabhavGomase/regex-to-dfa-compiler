#include "optimizer.h"
namespace {
using Candidate = std::pair<std::string, std::string>; // rule, expression
std::string replace(const ASTNode* root, const ASTNode* target, const std::string& value) {
    if (root == target) return "(" + value + ")";
    if (root->type == NodeType::EMPTY || root->type == NodeType::SYMBOL) return regexText(root);
    auto a = replace(root->left, target, value);
    switch (root->type) {
        case NodeType::UNION: return "(" + a + "|" + replace(root->right, target, value) + ")";
        case NodeType::CONCAT: return a + replace(root->right, target, value);
        case NodeType::STAR: return "(" + a + ")*";
        case NodeType::PLUS: return "(" + a + ")+";
        case NodeType::QUESTION: return "(" + a + ")?";
        default: throw std::runtime_error("Unknown rewrite node");
    }
}
void propose(const ASTNode* root, const ASTNode* node, std::vector<Candidate>& out) {
    if (!node) return;
    auto add = [&](const std::string& rule, const std::string& value) { out.push_back({rule, replace(root, node, value)}); };
    auto a = node->left;
    auto b = node->right;
    if (node->type == NodeType::UNION && regexText(a) == regexText(b)) add("Duplicate union: R|R -> R", regexText(a));
    if (node->type == NodeType::CONCAT) {
        if (a->type == NodeType::EMPTY) add("Left epsilon identity", regexText(b));
        if (b->type == NodeType::EMPTY) add("Right epsilon identity", regexText(a));
        if (a->type == NodeType::STAR && b->type == NodeType::STAR && regexText(a) == regexText(b))
            add("Adjacent identical stars: R*R* -> R*", regexText(a));
    }
    if (node->type == NodeType::STAR || node->type == NodeType::PLUS || node->type == NodeType::QUESTION) {
        if (a->type == NodeType::EMPTY) add("Repetition of epsilon", "()");
        if (a->type == NodeType::STAR) add("Repetition of a star language", regexText(a));
        if (node->type == NodeType::STAR && (a->type == NodeType::PLUS || a->type == NodeType::QUESTION))
            add("Star absorbs plus/optional", "(" + regexText(a->left) + ")*");
        if (node->type == NodeType::PLUS && a->type == NodeType::PLUS) add("Nested plus", regexText(a));
        if (node->type == NodeType::QUESTION && a->type == NodeType::QUESTION) add("Nested optional", regexText(a));
    }
    propose(root, a, out);
    propose(root, b, out);
}
DFA language(const std::string& text, const Limits& limits) {
    auto ast = parseRegex(text, limits);
    return buildDFA(buildNFA(ast.get(), limits), limits);
}
}
EquivalenceResult verifyRewrite(const std::string& before, const std::string& after, const Limits& limits) {
    auto a = language(before, limits), b = language(after, limits);
    return equivalentDFA(a, b, limits);
}
OptimizationResult optimizeRegex(const std::string& regex, const Limits& limits) {
    OptimizationResult result{regex, regex, {}};
    WorkBudget budget(limits);
    auto ast = parseRegex(regex, limits);
    while (true) {
        budget.check();
        std::vector<Candidate> candidates;
        propose(ast.get(), ast.get(), candidates);
        bool changed = false;
        for (const auto& candidate : candidates) {
            budget.check();
            // Serialization adds grouping. Skip any candidate exceeding the public input cap.
            if (candidate.second.size() > limits.regexBytes) continue;
            auto next = parseRegex(candidate.second, limits);
            int before = nodeCount(ast.get()), after = nodeCount(next.get());
            if (after >= before) continue;
            auto proof = verifyRewrite(result.optimized, candidate.second, limits);
            if (!proof.equivalent) continue;
            std::string text = regexText(next.get());
            result.rewrites.push_back({candidate.first, result.optimized, text, before, after, proof});
            result.optimized = text;
            ast = std::move(next);
            changed = true;
            break;
        }
        if (!changed) break;
    }
    return result;
}
