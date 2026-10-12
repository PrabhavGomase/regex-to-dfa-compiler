#include "direct.h"
#include <queue>
namespace {
std::set<int> unite(std::set<int> a, const std::set<int>& b) {
    a.insert(b.begin(), b.end());
    return a;
}
void follow(DirectResult& r, const std::set<int>& from, const std::set<int>& to) {
    for (int p : from) r.followpos[p].insert(to.begin(), to.end());
}
PositionFacts visit(const ASTNode* node, DirectResult& r) {
    PositionFacts f{nodeName(node->type), false, {}, {}};
    if (node->type == NodeType::SYMBOL) {
        int p = static_cast<int>(r.symbols.size()) + 1;
        r.symbols[p] = node->value;
        r.followpos[p] = {};
        f.firstpos = f.lastpos = {p};
        f.node += " " + std::string(1, node->value);
    } else if (node->type == NodeType::EMPTY) {
        f.nullable = true;
    } else {
        auto a = visit(node->left, r);
        if (node->type == NodeType::UNION || node->type == NodeType::CONCAT) {
            auto b = visit(node->right, r);
            if (node->type == NodeType::UNION) {
                f.nullable = a.nullable || b.nullable;
                f.firstpos = unite(a.firstpos, b.firstpos);
                f.lastpos = unite(a.lastpos, b.lastpos);
            } else {
                f.nullable = a.nullable && b.nullable;
                f.firstpos = a.nullable ? unite(a.firstpos, b.firstpos) : a.firstpos;
                f.lastpos = b.nullable ? unite(a.lastpos, b.lastpos) : b.lastpos;
                follow(r, a.lastpos, b.firstpos);
            }
        } else {
            f.nullable = node->type != NodeType::PLUS || a.nullable;
            f.firstpos = a.firstpos;
            f.lastpos = a.lastpos;
            if (node->type != NodeType::QUESTION) follow(r, a.lastpos, a.firstpos);
        }
    }
    r.facts.push_back(f);
    return f;
}
}
DirectResult buildDirectDFA(const ASTNode* root, const Limits& limits) {
    if (!root) throw std::runtime_error("Cannot build direct DFA from empty AST");
    WorkBudget budget(limits);
    DirectResult r;
    auto f = visit(root, r);
    // A dedicated position ID represents #; no user character is reserved.
    r.endPosition = static_cast<int>(r.symbols.size()) + 1;
    r.followpos[r.endPosition] = {};
    follow(r, f.lastpos, {r.endPosition});
    std::set<int> start = f.firstpos;
    if (f.nullable) start.insert(r.endPosition);
    r.facts.push_back({"CONCAT with end marker #", false, start, {r.endPosition}});
    std::set<char> alphabet;
    for (auto p : r.symbols) alphabet.insert(p.second);
    r.dfa.alphabet.assign(alphabet.begin(), alphabet.end());
    std::map<std::set<int>, int> ids;
    auto add = [&](const std::set<int>& positions) {
        if (r.dfa.nfaSets.size() >= limits.dfaStates)
            throw std::runtime_error("Resource limit: direct DFA state budget exceeded");
        int id = static_cast<int>(r.dfa.nfaSets.size());
        r.dfa.nfaSets.push_back(positions);
        ids[positions] = id;
        if (positions.count(r.endPosition)) r.dfa.acceptStates.insert(id);
        return id;
    };
    r.dfa.start = add(start);
    for (size_t i = 0; i < r.dfa.nfaSets.size(); ++i) {
        budget.check();
        for (char c : r.dfa.alphabet) {
            std::set<int> target;
            for (int p : r.dfa.nfaSets[i])
                if (p != r.endPosition && r.symbols.at(p) == c)
                    target.insert(r.followpos[p].begin(), r.followpos[p].end());
            if (target.empty()) continue;
            auto found = ids.find(target);
            int id = found == ids.end() ? add(target) : found->second;
            r.dfa.transitions[static_cast<int>(i)][c] = id;
        }
    }
    return r;
}
