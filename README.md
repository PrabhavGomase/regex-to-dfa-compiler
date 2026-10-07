# Regex → DFA Compiler

A compiler-design / automata project written in C++ that takes a regular
expression and transforms it step by step into a minimized DFA, with a
web visualization planned as the final stage.

## Pipeline

```
Regex → Lexer → Parser → AST → Thompson ε-NFA → Subset Construction (DFA) → DFA Minimization
```

## Current status

| Stage | Status |
|---|---|
| Lexer | Done |
| Parser (recursive descent) | Done |
| AST | Done |
| Thompson ε-NFA | Done |
| Subset construction (DFA) | Done |
| DFA minimization | Done |
| String matcher | Planned |
| Performance / state analysis | Planned |
| Web frontend and visualization | Planned |

## Supported syntax

| Syntax | Meaning |
|---|---|
| `a` | literal character |
| `ab` | concatenation |
| `a\|b` | union (alternation) |
| `a*` | zero or more |
| `a+` | one or more |
| `a?` | zero or one |
| `( )` | grouping |
| `\x` | escaped character |

Precedence, highest to lowest: postfix operators (`*`, `+`, `?`),
concatenation, union.

Not supported: character classes (`[a-z]`), `.`, `{n,m}`, anchors,
backreferences, lookaround.

## Build

```powershell
g++ -std=c++17 src/main.cpp src/lexer/lexer.cpp src/parser/parser.cpp src/ast/ast.cpp src/nfa/nfa.cpp src/dfa/dfa.cpp src/minimization/minimizer.cpp -o compiler
```

## Usage

```powershell
./compiler
Enter regular expression: (a|b)*abb
```

The program prints the tokens, the AST, the ε-NFA, the DFA, and the
minimized DFA.

### Example: `(a|b)*abb`

- Tokens: `( a | b ) * a b b`
- ε-NFA: 14 states
- DFA: 5 states
- Minimized DFA: 4 states

## Repository structure

```
src/
  lexer/         tokenization
  parser/        recursive-descent parser
  ast/           AST nodes and printing
  nfa/           Thompson construction
  dfa/           ε-closure and subset construction
  minimization/  DFA minimization
  matcher/       string matching (planned)
frontend/        web UI (planned)
docs/            architecture notes
tests/           test cases
```

## Team

- Member 1: Lexer and Parser
- Member 2: AST and NFA
- Member 3: Integration, Frontend, and Testing

## License

See `LICENSE`.