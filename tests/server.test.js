const { test, before, after } = require('node:test');
const assert = require('node:assert/strict');
const { spawnSync } = require('node:child_process');
const path = require('node:path');
const { createServer } = require('../scripts/server');
let server, base;
before(async () => {
  server = createServer();
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  base = 'http://127.0.0.1:' + server.address().port;
});
after(() => new Promise(resolve => server.close(resolve)));
async function post(endpoint, data) {
  const response = await fetch(base + '/api/' + endpoint, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(data) });
  return { status: response.status, body: await response.json() };
}
test('actual compiler data and accepting/rejecting traces', async () => {
  const result = await post('compile', { regex: '(a|b)*abb', input: 'aabb' });
  assert.equal(result.status, 200);
  assert.equal(result.body.nfa.stateCount, 14);
  assert.equal(result.body.dfa.stateCount, 5);
  assert.equal(result.body.minimized.stateCount, 4);
  assert.equal(result.body.match.accepted, true);
  assert.equal(result.body.match.dfaStates.length, 5);
  assert.equal((await post('compile', { regex: 'ab', input: 'a' })).body.match.accepted, false);
});
test('empty expressions, spaces, quotes and backslashes round-trip through JSON', async () => {
  for (const [regex, input] of [['', ''], ['()', ''], ['a b', 'a b'], ['"', '"'], ['\\\\', '\\'], ['\\.', '.'], ['a\\|b', 'a|b']]) {
    const result = await post('compile', { regex, input });
    assert.equal(result.status, 200, JSON.stringify(result.body));
    assert.equal(result.body.regex, regex);
    assert.equal(result.body.match.input, input);
    assert.equal(result.body.match.accepted, true);
  }
});
test('position facts and provenance come from direct construction', async () => {
  const { body } = await post('compile', { regex: 'ab' });
  assert.deepEqual(body.positions.map(p => p.followpos), [[2], [3], []]);
  assert.equal(body.positions[2].symbol, null);
  assert.equal(body.direct.provenance, 'syntax-tree positions');
});
test('invalid and unsupported syntax produces errors', async () => {
  for (const regex of ['a|', '(a', '.', '[a-z]', '\\d', '\\']) {
    const result = await post('compile', { regex });
    assert.equal(result.status, 400);
    assert.match(result.body.error, /offset/);
  }
});
test('equivalence includes an empty-string counterexample', async () => {
  assert.deepEqual((await post('equivalent', { regex: 'a*', candidate: 'a+' })).body.counterexample, '');
  assert.equal((await post('equivalent', { regex: 'a|b', candidate: 'b|a' })).body.equivalent, true);
  assert.equal((await post('equivalent', { regex: 'a', candidate: 'b' })).body.counterexample, 'a');
});
test('optimizer returns checked reductions', async () => {
  const { status, body } = await post('optimize', { regex: '(a|a)*' });
  assert.equal(status, 200);
  assert.ok(body.rewrites.length > 0);
  for (const r of body.rewrites) { assert.ok(r.afterNodes < r.beforeNodes); assert.equal(r.proof.equivalent, true); }
});
test('comparison has repeated measured samples and structural equivalence', async () => {
  const { body } = await post('compare', { regex: '(a|b)*abb' });
  assert.equal(body.equivalent, true);
  assert.equal(body.samples.length, 20);
  assert.equal(body.warmups, 3);
  assert.equal(body.repetitions, 10);
  assert.equal(body.direct.minStates, body.subset.minStates);
  for (const s of body.samples) assert.ok(s.constructionUs >= 0 && s.minimizationUs >= 0);
});
test('input validation and bounded requests', async () => {
  for (const data of [null, [], {}, { regex: 123 }, { regex: 'a\nb' }, { regex: 'é' }, { regex: 'a'.repeat(257) }, { regex: 'a', input: 'a'.repeat(4097) }, { regex: 'a', candidate: null }])
    assert.equal((await post('compile', data)).status, 400);
  const res = await fetch(base + '/api/compile', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: 'x'.repeat(17000) });
  assert.equal(res.status, 413);
});
test('transport rejects malformed JSON, wrong media type, origins and missing routes', async () => {
  assert.equal((await fetch(base + '/api/compile', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: '{' })).status, 400);
  assert.equal((await fetch(base + '/api/compile', { method: 'POST', body: '{}' })).status, 415);
  assert.equal((await fetch(base, { headers: { Origin: 'https://example.com' } })).status, 403);
  assert.equal((await fetch(base + '/api/unknown')).status, 404);
  assert.equal((await fetch(base + '/src/main.cpp')).status, 404);
  assert.equal((await fetch(base + '/../../README.md')).status, 404);
});
test('frontend assets are served with the local compiler endpoint', async () => {
  for (const asset of ['/', '/script.js', '/style.css']) {
    const res = await fetch(base + asset);
    assert.equal(res.status, 200);
    assert.ok(res.headers.get('content-security-policy'));
    assert.ok((await res.text()).length > 100);
  }
});
test('recorded benchmarks contain all raw measured repetitions', async () => {
  const response = await fetch(base + '/api/benchmarks');
  assert.equal(response.status, 200);
  const data = await response.json();
  assert.equal(data.cases.length, 10);
  for (const c of data.cases) {
    assert.equal(c.samples.length, c.repetitions * 2);
    assert.equal(c.equivalent, true);
    assert.equal(c.subset.minStates, c.direct.minStates);
  }
});
test('CLI error and stdin protocol return complete JSON without shell interpretation', () => {
  const executable = path.resolve('build', process.platform === 'win32' ? 'regexlab.exe' : 'regexlab');
  const bad = spawnSync(executable, ['--json', 'a|'], { encoding: 'utf8', windowsHide: true });
  assert.equal(bad.status, 1);
  assert.match(JSON.parse(bad.stdout).error, /offset/);
  const good = spawnSync(executable, ['--request'], { input: 'compile\n"\n"\n\n', encoding: 'utf8', windowsHide: true });
  assert.equal(good.status, 0);
  assert.equal(JSON.parse(good.stdout).match.accepted, true);
});
