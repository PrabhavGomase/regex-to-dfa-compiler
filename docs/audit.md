# Initial audit and completion checklist

Audited against commit `3b95846` on 13 October 2026. The working tree was clean on `main`, with an existing GitHub origin.

The original build ran successfully with GCC 6.3 outside the Windows sandbox. `(a|b)*abb` produced 14 NFA states and 5 DFA states. `a|` failed with a generic parser error; `.` was silently compiled as a literal despite the documented unsupported syntax.

| Area | Verified initial state | Priority |
|---|---|---|
| Lexer/parser/AST | Implemented; unsupported syntax accepted, spaces discarded, AST allocations leaked | 1 |
| Thompson and subset | Working reference output; no resource limits | 1 |
| Minimization/matching | All four source/header files empty | 1 |
| Tests | Manual examples only; no executable tests | 1 |
| Direct DFA/equivalence/optimization | Absent | 2 |
| Frontend | Static saved example; disabled matching; no compiler connection | 2 |
| Benchmarks | Absent | 2 |
| Documentation | README incorrectly said minimization was done; architecture stale | 3 |
| Publication | Existing origin; authentication not yet checked | 3 |

Completion order: syntax and ownership; matching/minimization and resource guards; direct construction and equivalence; verified optimization; automated tests; connect the existing frontend; repeated benchmarks; documentation, review, commit, push.
