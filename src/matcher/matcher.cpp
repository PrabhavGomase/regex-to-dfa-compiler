#include "matcher.h"
namespace {
void checkInput(const std::string& input) {
    if (input.size() > 4096) throw std::runtime_error("Resource limit: test string exceeds 4096 bytes");
    for (unsigned char c : input)
        if (c < 32 || c > 126) throw std::runtime_error("Test strings must be printable ASCII");
}
}
DFATrace traceDFA(const DFA& dfa, const std::string& input) {
    checkInput(input);
    DFATrace trace{false, {dfa.start}};
    int state = dfa.start;
    for (char c : input) {
        state = nextState(dfa, state, c);
        trace.states.push_back(state);
    }
    trace.accepted = dfa.acceptStates.count(state) != 0;
    return trace;
}
NFATrace traceNFA(const NFA& nfa, const std::string& input) {
    checkInput(input);
    NFATrace trace{false, {epsilonClosure(nfa, {nfa.start})}};
    for (char c : input)
        trace.states.push_back(epsilonClosure(nfa, moveOnSymbol(nfa, trace.states.back(), c)));
    trace.accepted = trace.states.back().count(nfa.accept) != 0;
    return trace;
}
bool matchDFA(const DFA& dfa, const std::string& input) { return traceDFA(dfa, input).accepted; }
bool matchNFA(const NFA& nfa, const std::string& input) { return traceNFA(nfa, input).accepted; }
