# Tests

Manual tests. Each regex is entered into `./compiler` and the output is checked by hand.

## Valid regexes

All of these ran through Lexer → Parser → AST → ε-NFA → DFA and produced output.

**Basic**
- `a`
- `b`
- `ab`
- `24`

**Union and grouping**
- `a|b`
- `(a|b)`
- `(a|b)(c|d)`

**Postfix operators**
- `a*`
- `a+`
- `a?`

**Nested**
- `(a|b)*`
- `((a*)*)*`
- `a(b|c)*d`
- `(a|b)*abb`
- `(a|b)*abb(a|b)`

## Reference case: `(a|b)*abb`

- Tokens: 10, including END
- AST: `CONCAT` nested to the left, with `STAR` over `UNION(a, b)` as the deepest branch
- ε-NFA: 14 states, start q6, accept q13
- DFA: 5 states, one accepting state

## Precedence checks

- `ab*` gives `CONCAT(a, STAR(b))`
- `a|b*` gives `UNION(a, STAR(b))`
- `a+b?` gives `CONCAT(PLUS(a), QUESTION(b))`

## Stress cases

- `(a*)*` and `((a*)*)*`: nested stars, ε-closure terminates
- `(a*|b*)*`: several ε-cycles
- `(a?)+`: optional inside plus
- `a**`: repeated postfix operators
- `a\*b` and `\(a\)`: escaped operators are treated as literals

## Invalid regexes

Each of these is rejected with a syntax error:

- `(` → Expected literal or '('
- `a|` → Expected literal or '('
- `*a` → Expected literal or '('
- `()` → Expected literal or '('
- `a)` → Unexpected token after expression
- `(a` → Missing closing parenthesis

## What these tests cover

These tests check that each stage runs and that the printed tokens, AST, NFA and DFA have the expected shape. They do not yet check that the DFA accepts exactly the right strings. That needs the matcher, which is planned.

## Known limitations

- `[a-z]`, `.`, `{n}`, `^`, `$`, `\d` are not supported and are read as literal characters.
- Empty regex, `()`, and empty alternatives such as `a|` or `(a|)` are rejected.
- Non-ASCII characters are handled byte by byte.

## Planned

- Matcher tests with strings that should be accepted and rejected
- Automated test runner
- Minimization tests