// Dependency-free build entry point. CXX may select another compiler executable.
const fs = require('node:fs');
const path = require('node:path');
const { spawnSync } = require('node:child_process');
const root = path.resolve(__dirname, '..');
const sources = ['lexer/lexer', 'parser/parser', 'ast/ast', 'nfa/nfa', 'dfa/dfa',
  'minimization/minimizer', 'matcher/matcher', 'direct/direct', 'analysis/equivalence',
  'analysis/optimizer', 'compiler/compiler', 'analysis/benchmark', 'compiler/json'];
const suffix = process.platform === 'win32' ? '.exe' : '';
fs.mkdirSync(path.join(root, 'build'), { recursive: true });
function build(main, name) {
  const result = spawnSync(process.env.CXX || 'g++', ['-std=c++17', '-O2', '-Wall', '-Wextra', '-pedantic',
    main, ...sources.map(s => 'src/' + s + '.cpp'), '-o', 'build/' + name + suffix],
    { cwd: root, stdio: 'inherit' });
  if (result.error) throw result.error;
  if (result.status !== 0) process.exit(result.status || 1);
}
build('src/main.cpp', 'regexlab');
build('tests/compiler_tests.cpp', 'compiler_tests');
const compiler = spawnSync(process.env.CXX || 'g++', ['--version'], { encoding: 'utf8', windowsHide: true });
fs.writeFileSync(path.join(root, 'build/build-info.json'), JSON.stringify({ compiler: compiler.stdout?.split(/\r?\n/)[0] || 'unknown', flags: '-std=c++17 -O2 -Wall -Wextra -pedantic', builtAt: new Date().toISOString() }, null, 2));
console.log('Built compiler and correctness tests.');
