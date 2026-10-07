# Tests

Manual tests, run by entering each regex into `./compiler`.

## Valid regexes

| Regex | Expected result |
|---|---|
| `(a\|b)*abb` | 10 tokens including END; AST as in `docs/architecture.md`; NFA with 14 states, start q6, accept q13 |
| `ab*` | `CONCAT` of `SYMBOL a` and `STAR(SYMBOL b)` |
| `a\|b*` | `UNION` of `SYMBOL a` and `STAR(SYMBOL b)` |
| `a+b?` | `CONCAT` of `PLUS(SYMBOL a)` and `QUESTION(SYMBOL b)` |

## Invalid regexes

| Regex | Expected error |
|---|---|
| `(` | Expected literal or '(' |
| `a\|` | Expected literal or '(' |
| `*a` | Expected literal or '(' |
| `()` | Expected literal or '(' |
| `a)` | Unexpected token after expression |
| `(a` | Missing closing parenthesis |

## Status
All cases above pass. Automated tests and DFA / matcher tests are planned.