const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');
const { spawnSync } = require('node:child_process');
const root = path.resolve(__dirname, '..');
const repetitions = process.argv[2] || '20';
if (!/^\d+$/.test(repetitions) || Number(repetitions) < 1 || Number(repetitions) > 100) throw new Error('Use 1–100 repetitions');
const executable = path.join(root, 'build', process.platform === 'win32' ? 'regexlab.exe' : 'regexlab');
const result = spawnSync(executable, ['--benchmark', repetitions], { cwd: root, encoding: 'utf8', windowsHide: true, timeout: 120000, maxBuffer: 16 * 1024 * 1024 });
if (result.error) throw result.error;
if (result.status !== 0) { console.error(result.stdout || result.stderr); process.exit(1); }
const data = JSON.parse(result.stdout);
let buildInfo = { compiler: 'unknown (not built by scripts/build.js)', flags: 'unknown' };
try { buildInfo = JSON.parse(fs.readFileSync(path.join(root, 'build/build-info.json'), 'utf8')); } catch {}
data.recordedAt = new Date().toISOString();
data.environment = { os: os.platform() + ' ' + os.release(), arch: os.arch(), cpu: os.cpus()[0]?.model || 'unknown', ...buildInfo, timer: process.platform === 'win32' ? 'QueryPerformanceCounter' : 'steady_clock' };
data.methodology = 'Shared parsed AST; three warmups per method; alternating method order; high-resolution monotonic timer in microseconds (QPC on Windows, steady_clock elsewhere); NFA+subset versus direct position method, including tutor metadata. Minimize separately. Parsing, JSON, process startup, and equivalence checks excluded. Complete minimized DFA counts include reachable sinks.';
fs.writeFileSync(path.join(root, 'docs/benchmark-results.json'), JSON.stringify(data, null, 2) + '\n');
const mean = values => values.reduce((a, b) => a + b, 0) / values.length;
const summary = data.cases.map(c => {
  const subset = c.samples.filter(s => s.method === 'thompson_subset'), direct = c.samples.filter(s => s.method === 'direct');
  return [c.regex, c.nfaStates, c.subset.states, c.direct.states, c.subset.minStates,
    mean(subset.map(s => s.constructionUs)).toFixed(2), mean(direct.map(s => s.constructionUs)).toFixed(2),
    mean(subset.map(s => s.minimizationUs)).toFixed(2), mean(direct.map(s => s.minimizationUs)).toFixed(2), c.equivalent];
});
const markdown = '# Recorded benchmarks\n\nRecorded: ' + data.recordedAt + '\n\n' + JSON.stringify(data.environment) + '\n\n' + data.methodology + '\n\n'
  + '| Regex | NFA | Subset DFA | Direct DFA | Min DFA | Subset build µs | Direct build µs | Subset minimize µs | Direct minimize µs | Equivalent |\n'
  + '|---|---:|---:|---:|---:|---:|---:|---:|---:|---|\n'
  + summary.map(row => '| ' + row.map((cell, i) => i === 0 ? '`' + String(cell).replace(/\|/g, '\\|') + '`' : cell).join(' | ') + ' |').join('\n')
  + '\n\nRaw per-repetition samples and transition counts: [benchmark-results.json](benchmark-results.json). Timing varies with machine and load; these are measurements of this implementation, not asymptotic claims. State-growth cases remember the fourth, sixth, and eighth symbol from the end.\n';
fs.writeFileSync(path.join(root, 'docs/benchmarks.md'), markdown);
console.log(markdown);
