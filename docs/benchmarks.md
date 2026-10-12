# Recorded benchmarks

Recorded: 2026-10-12T20:51:34.508Z

{"os":"win32 10.0.26200","arch":"x64","cpu":"11th Gen Intel(R) Core(TM) i5-1135G7 @ 2.40GHz","compiler":"g++ (MinGW.org GCC-6.3.0-1) 6.3.0","flags":"-std=c++17 -O2 -Wall -Wextra -pedantic","builtAt":"2026-10-12T20:51:31.882Z","timer":"QueryPerformanceCounter"}

Shared parsed AST; three warmups per method; alternating method order; high-resolution monotonic timer in microseconds (QPC on Windows, steady_clock elsewhere); NFA+subset versus direct position method, including tutor metadata. Minimize separately. Parsing, JSON, process startup, and equivalence checks excluded. Complete minimized DFA counts include reachable sinks.

| Regex | NFA | Subset DFA | Direct DFA | Min DFA | Subset build µs | Direct build µs | Subset minimize µs | Direct minimize µs | Equivalent |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|
| `a` | 2 | 2 | 2 | 3 | 7.01 | 4.75 | 5.79 | 5.93 | true |
| `ab` | 4 | 3 | 3 | 4 | 12.52 | 7.45 | 11.06 | 11.33 | true |
| `a\|b\|c\|d` | 14 | 5 | 2 | 3 | 23.21 | 16.27 | 11.51 | 7.22 | true |
| `(a\|b)*abb` | 14 | 5 | 4 | 4 | 35.51 | 25.00 | 11.59 | 9.47 | true |
| `((a*)*)*` | 8 | 2 | 1 | 1 | 10.06 | 6.75 | 3.37 | 2.23 | true |
| `a?b+c*` | 12 | 4 | 4 | 5 | 19.13 | 15.90 | 12.01 | 12.08 | true |
| `(a\|b\|c)*abc` | 18 | 6 | 4 | 4 | 63.90 | 38.74 | 14.95 | 11.12 | true |
| `(a\|b)*a(a\|b)(a\|b)(a\|b)` | 28 | 17 | 16 | 16 | 204.01 | 112.97 | 61.70 | 62.70 | true |
| `(a\|b)*a(a\|b)(a\|b)(a\|b)(a\|b)(a\|b)` | 40 | 65 | 64 | 64 | 1303.79 | 517.84 | 332.65 | 314.08 | true |
| `(a\|b)*a(a\|b)(a\|b)(a\|b)(a\|b)(a\|b)(a\|b)(a\|b)` | 52 | 257 | 256 | 256 | 7358.65 | 2891.82 | 2033.53 | 2082.97 | true |

Raw per-repetition samples and transition counts: [benchmark-results.json](benchmark-results.json). Timing varies with machine and load; these are measurements of this implementation, not asymptotic claims. State-growth cases remember the fourth, sixth, and eighth symbol from the end.
