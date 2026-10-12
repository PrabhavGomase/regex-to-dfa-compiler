# Verification

Run from the repository root:

```sh
node scripts/build.js
node scripts/test.js
```

The runner invokes the compiled C++ correctness suite, then Node's built-in test runner for the actual compiler HTTP/CLI bridge. No third-party testing packages are required.

## Coverage

- Operators, precedence, grouping, nested/repeated postfix operators.
- Escaped punctuation, spaces, quotes/backslashes, and end-marker collision.
- Empty expression, explicit ε, nullable plus/optional, accepting and rejecting examples.
- Syntax offsets and rejected unsupported features, ASCII and byte limits.
- Exhaustive words of length up to four over small alphabets, using independent `std::regex_match` where syntax overlaps.
- A deterministic grammar-generated expression corpus and matching agreement across ε-NFA, subset DFA, direct DFA, and both minimized DFAs.
- Structural equivalence, unreachable-state elimination, missing-edge completion, explicit/implicit dead-state merging, empty alphabet, and all-accepting/all-rejecting DFAs.
- Shortest counterexamples, including ε, and witness validity.
- Every built-in optimization family; proof and AST decrease for every accepted step; invalid user candidates.
- DFA/NFA traces, construction/product budgets, deadlines, and actual exponential-family state caps for both construction methods.
- HTTP/CLI JSON serialization, all compiler endpoints, repeated comparison samples, saved benchmark data, transport/input validation, fixed static asset serving, and error statuses.

Tests fail immediately on disagreement; nothing is skipped. The tests do not prove correctness for every possible regex. Product traversal establishes language equivalence for each compiled comparison within the configured bounds; the independent oracle guards against both implementations sharing a mistaken language interpretation.

## Local verification (13 October 2026)

The final test counts and commands are recorded in [delivery notes](../docs/delivery.md). GCC 6.3 on Windows compiled with C++17, optimization, and warnings. Node HTTP tests start a server on an ephemeral loopback port and close it afterward.

Browser checks exercised the real local interface: accepted `aabb` for `(a|b)*abb`, minimized/direct diagrams, nullable/firstpos/lastpos/followpos, repeated timings, saved benchmarks, duplicate-union simplification proof, `a*` versus `a+` with an ε witness, invalid-input clearing, keyboard stage navigation, and large-DFA transition tables. No browser script warnings/errors were reported in the checked session. [Screenshot](../docs/screenshots/workbench.jpg) is an actual browser capture.

GitHub CI is configured to build and run the same automated tests on Ubuntu. Local success does not by itself claim that a remote CI run has passed.

## Benchmarks

```sh
node scripts/benchmark.js 20
```

This overwrites the deliberately tracked raw measurements and generated Markdown table. Each regex receives three warmups and twenty recorded runs per method. See [results](../docs/benchmarks.md) for environment details and methodology.
