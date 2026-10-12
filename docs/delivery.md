# Implementation and verification record

Completed locally on 13 October 2026, extending the original C++ modules and vanilla frontend.

## Original status

The clean starting repository at `3b95846` implemented lexer, parser, AST, Thompson NFA, and subset DFA construction. The minimizer/matcher files were empty; direct construction, equivalence, optimization, benchmarks, and executable tests were absent. The frontend displayed saved data for one expression. The README overstated minimization completion. See [audit](audit.md).

## Completed work

- Explicit positioned syntax diagnostics, printable-ASCII/escape rules, ε expressions, and owning AST cleanup.
- DFA/NFA matching and traces, complete reachable minimization, direct position DFA with tutor facts.
- Combined-alphabet product equivalence and shortest counterexamples, including ε.
- Reducing local optimizations checked individually by language-equivalence proofs.
- State/product/time/input budgets and a bounded local compiler bridge.
- Live compiler UI, stage navigation, execution highlighting, large-state tables, measured comparison, optimization/candidate verification, and recorded benchmark views.
- Reproducible build/test/benchmark commands, optional CMake configuration, CI workflow, architecture/syntax/limitations documentation, and actual browser screenshots.

## Final automated verification

Commands:

```sh
node scripts/build.js
node scripts/test.js
node scripts/benchmark.js 20
```

Final correctness suite: **26,739 passing assertions**. HTTP/CLI integration suite: **12 tests passed, 0 failed, 0 skipped**. The warning-enabled GCC 6.3 build completed successfully. CMake was not installed, so its optional build path was not executed locally.

Browser checks verified acceptance, direct position analysis, proof-backed simplification, the empty-string counterexample, invalid-input clearing, keyboard stages, repeated comparisons, benchmark data, and a 65-state table fallback. No script warnings/errors were reported during inspection.

## Recorded benchmark results

Ten expressions, three warmups, twenty recorded runs per method: **400 raw construction/minimization samples**, with equivalence established for every case. Measurements use QueryPerformanceCounter on the recorded Windows machine.

For `(a|b)*abb`, recorded construction means were **35.51 µs** (Thompson + subset) and **25.00 µs** (direct). State counts were 14 NFA, 5 subset DFA, 4 direct DFA, and 4 for either minimized DFA. The largest recorded growth case produced 257 subset states, 256 direct states, and 256 minimized states, with means **7,358.65 µs** and **2,891.82 µs** respectively.

See [measured tables](benchmarks.md) and [raw samples](benchmark-results.json) for all results and environment details. These are experimental observations, not portable speed guarantees.

## Running and limits

Run `node scripts/server.js` and open http://127.0.0.1:3000. Run `./build/regexlab` (`.exe` on Windows) for the interactive CLI.

Remaining deliberate limitations: printable ASCII; no classes/wildcard/counts/anchors/backreferences/lookaround; educational Moore refinement and scan-based closures; bounded inputs/states/proofs; a finite simplification ruleset; deterministic graph layout with table fallback. GitHub publishes the repository, not an externally hosted compiler service. See the [README](../README.md) for exact syntax, commands, limits, and architecture.
