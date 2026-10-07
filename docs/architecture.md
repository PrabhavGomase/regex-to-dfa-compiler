# Architecture

## Pipeline (Review 1 scope)

```
Regex string → Lexer → Tokens → Parser → AST → Thompson construction → ε-NFA
```

Later stages (subset construction, minimization, matching, analysis,
frontend) are planned and not part of this review.

## Supported syntax

| Syntax | Meaning |
|---|---|
| `a` | literal |
| `ab` | concatenation |
| `a\|b` | union |
| `a*` | zero or more |
| `a+` | one or more |
| `a?` | zero or one |
| `( )` | grouping |
| `\x` | escaped character |

Not supported: character classes, `.`, `{n,m}`, anchors, backreferences,
lookaround.

## Grammar

```
Expression → Term ( '|' Term )*
Term       → Factor Factor*
Factor     → Primary ( '*' | '+' | '?' )*
Primary    → LITERAL | '(' Expression ')'
```

Precedence, highest to lowest: postfix operators, concatenation, union.

## Modules

### Lexer (`src/lexer/`)
Converts the regex string into tokens: `LITERAL`, `OR`, `STAR`, `PLUS`,
`QUESTION`, `LPAREN`, `RPAREN`, `END`. Skips spaces and handles escaped
characters.

### Parser (`src/parser/`)
Recursive-descent parser, one function per grammar rule. Builds the AST
and throws a `std::runtime_error` on invalid input:

- `Expected literal or '('`
- `Missing closing parenthesis`
- `Unexpected token after expression`

### AST (`src/ast/`)
A binary tree of `ASTNode` with types `SYMBOL`, `UNION`, `CONCAT`,
`STAR`, `PLUS`, `QUESTION`. Unary nodes use only the left child.
`printAST` prints the tree with indentation.

Example, `(a|b)*abb`:

```
CONCAT
  CONCAT
    CONCAT
      STAR
        UNION
          SYMBOL a
          SYMBOL b
      SYMBOL a
    SYMBOL b
  SYMBOL b
```

### NFA (`src/nfa/`)
Thompson's construction. Each AST node becomes a fragment with one start
state and one accept state. All fragments share one transition list.
ε is stored as `'\0'`.

| AST node | Construction |
|---|---|
| `SYMBOL a` | `s --a--> f` |
| `CONCAT(A,B)` | `A.accept --ε--> B.start` |
| `UNION(A,B)` | new `s`, `f`; `s --ε--> A.start`, `s --ε--> B.start`, `A.accept --ε--> f`, `B.accept --ε--> f` |
| `STAR(A)` | new `s`, `f`; `s --ε--> A.start`, `s --ε--> f`, `A.accept --ε--> A.start`, `A.accept --ε--> f` |
| `PLUS(A)` | like star without `s --ε--> f` |
| `QUESTION(A)` | new `s`, `f`; `s --ε--> A.start`, `s --ε--> f`, `A.accept --ε--> f` |

Data structures:

```cpp
struct Transition { int from; int to; char symbol; };
struct NFA {
    int start;
    int accept;
    int stateCount;
    std::vector<Transition> transitions;
};
```

For `(a|b)*abb` the NFA has 14 states, start `q6`, accept `q13`.

## Integration (`src/main.cpp`)
Reads a regex, runs lexer, parser and NFA builder in order, and prints
the tokens, the AST and the NFA. Errors from any stage are caught and
printed.

## Planned stages
Subset construction (DFA), DFA minimization, string matcher,
performance analysis, web visualization.