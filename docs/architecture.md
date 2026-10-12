# Compiler architecture

## Integrated pipeline

```text
regex → positioned tokens → owning AST → Thompson ε-NFA → subset DFA → complete minimized DFA
                              └──────→ position analysis → direct DFA → complete minimized DFA
```

`compileRegex` builds both paths and checks both construction equivalence and minimization preservation through product-DFA traversal. Errors propagate to the CLI or HTTP response. A failed proof is an error, never a successful comparison.

## Syntax and ownership

The lexer rejects unsupported constructs, non-printable/non-ASCII bytes, and ambiguous shorthand escapes. Positions are zero-based byte offsets. Spaces are literal symbols. The parser's grammar is:

```text
Expression → Term ('|' Term)*
Term       → Factor Factor*
Factor     → Primary ('*' | '+' | '?')*
Primary    → LITERAL | '(' Expression ')' | '(' ')'
```

A completely empty input becomes EMPTY; empty alternatives are errors. EMPTY denotes ε, not the empty language. AST children are owned by their parent and deleted recursively. Parser-local unique pointers free partial trees on syntax failures. Public parsing returns an owning unique pointer through `parseRegex`; the original `Parser::parse` API transfers a raw pointer to its caller for compatibility.

## Thompson and subset construction

SYMBOL creates a two-state fragment. EMPTY creates an ε edge. Concatenation connects fragments with ε; union adds branching and joining states. STAR adds a bypass and a return edge; PLUS omits the bypass; QUESTION omits the return edge.

Subset construction begins with ε-closure of the NFA start. For each alphabet symbol it computes move followed by ε-closure. New nonempty sets are queued. Empty destinations are omitted and mean rejection. The retained `nfaSets` field records NFA-state provenance.

## Direct position construction

Each literal occurrence receives its own position, including repeated characters. EMPTY is nullable with empty first/last sets.

| Node | Nullable | Firstpos | Lastpos | Follow updates |
|---|---|---|---|---|
| UNION(A,B) | A or B | union | union | None |
| CONCAT(A,B) | A and B | A, plus B if A nullable | B, plus A if B nullable | Each A.last → B.first |
| STAR(A) | true | A.first | A.last | Each A.last → A.first |
| PLUS(A) | A.nullable | A.first | A.last | Each A.last → A.first |
| QUESTION(A) | true | A.first | A.last | None |

A dedicated end-marker position is appended through a conceptual concatenation. It has **no character value**, so user `#` literals cannot collide with it. The initial DFA set is the augmented tree's firstpos. On symbol c, union followpos for positions labeled c. A state accepts iff it contains the end position. Tutor records expose postorder node facts and all follow sets; direct DFA backing sets are positions.

## Complete minimization

Moore refinement first explores reachable states, treating every missing transition as a shared implicit rejecting sink (-1). It partitions by acceptance, then repeatedly groups signatures of the current block and successor blocks until stable. Each block becomes one state in a complete DFA. Unreachable input states are excluded, and explicit dead states merge with the implicit sink when equivalent.

Minimized backing sets contain **source DFA IDs**, not NFA states. A sink may increase the count compared with a partial input; state-count reduction is reported against the partial input with this caveat.

## Matching and proofs

DFA matching starts at the start state and follows one edge per character, retaining -1 after a missing edge. NFA simulation starts at ε-closure and performs move/closure per character. Both accept only at the end of the full input. Traces retain the initial state/set and one state/set after each consumed symbol.

Product equivalence uses the union of both alphabets. Missing edges in either DFA go to an implicit rejecting sink. Breadth-first traversal checks whether acceptance differs at each pair. Predecessor links reconstruct a shortest witness; the initial pair may already differ, yielding the **empty string**. An empty witness must not be confused with success. `equivalent` is the authority; JSON uses null only for successful proofs.

The optimizer proposes one local tree rewrite at a time, serializes/reparses the candidate, rejects candidates with no AST-node decrease, and checks language equivalence. Accepted steps carry the rule, both expressions, before/after counts, and visited product pairs. The strictly decreasing node count guarantees termination; a deadline bounds work further. Oversized serialized candidates are skipped. User-submitted candidates are checked through the same proof API.

## Measurements versus complexity

Repeated comparison parses once, shares that AST between methods, performs three warmups, alternates order, and records construction and minimization durations separately. Thompson timing includes NFA creation; direct timing includes syntax-tree position analysis and tutor metadata. Neither timing includes parsing, proof traversal, serialization, or process startup. Windows uses QueryPerformanceCounter to avoid older MinGW's coarse standard clock. Other platforms use steady_clock. Build metadata and machine details accompany the saved raw samples.

Theory: Thompson NFA size is linear in the AST. Subset DFA states can grow exponentially in NFA size; position-subset states can also grow exponentially in the number of literal positions. Equivalence can visit the product of the two state counts. Moore refinement can require linear-many refinement rounds; this map-based implementation is not Hopcroft minimization. ε-closure scans the transition list for each visited state, so it prioritizes readability over indexed-graph performance. Recorded microsecond timings describe this implementation on the recorded machine, not those worst-case bounds.

## Local integration

Node's built-in HTTP server serves a fixed asset allowlist and validates printable-ASCII JSON requests. It invokes the compiler directly with no shell and sends values over stdin, avoiding command-line quoting issues. Compilation concurrency, input/output sizes, and timeouts are bounded. The server binds to 127.0.0.1, checks Host and Origin, and serves a same-origin content security policy.

The frontend displays actual compiler results. Editing a regex clears stale automata and disables dependent actions until compilation succeeds. The trace slider highlights active graph states. Large automata expose state filters and transition/provenance tables; the CLI JSON retains every edge.

## Resource budgets

Defaults are in `src/limits.h`. Lexer byte caps bound tree depth. A conservative NFA estimate is checked before allocation; subset/direct builders check before inserting states; minimization allows one completion sink; product traversal checks before adding pairs. Cooperative deadlines are checked during discovery/refinement/proof loops. The HTTP process timeout remains an outer bound for the combined pipeline.
