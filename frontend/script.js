'use strict';
const $ = id => document.getElementById(id);
const SVG_NS = 'http://www.w3.org/2000/svg';
let current = null, busy = false, generation = 0;
const actionIds = ['test', 'compare', 'optimize', 'equivalent'];
function element(name, text, cls) {
  const e = document.createElement(name);
  if (text !== undefined) e.textContent = text;
  if (cls) e.className = cls;
  return e;
}
function svgElement(name, attrs, parent) {
  const e = document.createElementNS(SVG_NS, name);
  for (const [key, value] of Object.entries(attrs)) e.setAttribute(key, value);
  if (parent) parent.append(e);
  return e;
}
function displaySymbol(symbol) { return symbol === null ? 'ε' : symbol === ' ' ? '␠' : symbol; }
function refreshActions() {
  const valid = current && current.regex === $('regex').value;
  for (const id of actionIds) $(id).disabled = busy || !valid;
  $('compile').disabled = busy;
}
async function api(operation, data) {
  const response = await fetch('/api/' + operation, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(data) });
  const body = await response.json();
  if (!response.ok || body.error) throw new Error(body.error || 'Request failed');
  return body;
}
async function perform(task) {
  if (busy) return;
  busy = true; refreshActions();
  $('status').textContent = 'Working…';
  try { await task(); $('status').textContent = ''; }
  catch (error) { $('status').textContent = error.message; }
  finally { busy = false; refreshActions(); }
}
function table(headers, rows, caption) {
  const wrap = element('div', undefined, 'table-wrap');
  const t = element('table');
  if (caption) t.append(element('caption', caption));
  const head = element('thead'), header = element('tr');
  for (const h of headers) header.append(element('th', h));
  head.append(header); t.append(head);
  const body = element('tbody');
  for (const row of rows) {
    const tr = element('tr');
    for (const value of row) tr.append(element('td', value));
    body.append(tr);
  }
  t.append(body); wrap.append(t); return wrap;
}
function renderTokens(tokens) {
  $('token-list').replaceChildren(...tokens.map(t => {
    const li = element('li', undefined, 'token' + (t.type === 'LITERAL' ? ' literal' : ''));
    li.append(element('span', t.type === 'END' ? 'end' : displaySymbol(t.value), 'tk-value'), element('span', t.type + ' @' + t.offset, 'tk-type'));
    return li;
  }));
}
function astLines(node, depth = 0) {
  if (!node) return '';
  return '  '.repeat(depth) + node.type + (node.value === null ? '' : ' ' + JSON.stringify(node.value)) + '\n' + astLines(node.left, depth + 1) + astLines(node.right, depth + 1);
}
function renderAST(root) {
  const container = $('ast-content'); container.replaceChildren();
  let nextX = 0, maxDepth = 0;
  const nodes = [], edges = [];
  function visit(node, depth) {
    if (!node) return null;
    const a = visit(node.left, depth + 1), b = visit(node.right, depth + 1);
    const children = [a, b].filter(Boolean);
    const x = children.length ? children.reduce((s, c) => s + c.x, 0) / children.length : 70 + nextX++ * 110;
    const n = { node, x, y: 35 + depth * 65 };
    nodes.push(n); maxDepth = Math.max(maxDepth, depth);
    for (const child of children) edges.push([n, child]);
    return n;
  }
  visit(root, 0);
  if (nodes.length <= 80) {
    const wrap = element('div', undefined, 'diagram'), svg = svgElement('svg', { viewBox: '0 0 ' + Math.max(220, nextX * 110 + 50) + ' ' + (maxDepth * 65 + 80), role: 'img', 'aria-label': 'Abstract syntax tree' });
    for (const [a, b] of edges) svgElement('line', { x1: a.x, y1: a.y, x2: b.x, y2: b.y, stroke: '#7b8a94' }, svg);
    for (const n of nodes) {
      svgElement('rect', { x: n.x - 45, y: n.y - 14, width: 90, height: 28, rx: 3, fill: n.node.type === 'SYMBOL' ? '#d9ecee' : '#fbfcfb', stroke: '#14232f' }, svg);
      svgElement('text', { x: n.x, y: n.y + 4, 'text-anchor': 'middle', 'font-size': 12 }, svg).textContent = n.node.value === null ? n.node.type : JSON.stringify(n.node.value);
    }
    wrap.append(svg); container.append(wrap);
  } else container.append(element('p', 'Large AST: use the complete text tree below.'));
  const details = element('details'); details.append(element('summary', 'Text tree'), element('pre', astLines(root)));
  container.append(details);
}
function drawAutomaton(a, kind) {
  const wrap = element('div', undefined, 'diagram');
  // Breadth-first layers provide a deterministic general layout. Tables handle dense/large cases.
  if (a.stateCount > 40 || a.transitions.length > 100) {
    wrap.append(element('p', 'Large automaton (' + a.stateCount + ' states). Inspect the searchable transitions and state sets below.'));
    return wrap;
  }
  const levels = new Map([[a.start, 0]]), queue = [a.start];
  for (let i = 0; i < queue.length; i++) for (const edge of a.transitions.filter(e => e.from === queue[i]))
    if (!levels.has(edge.to)) { levels.set(edge.to, levels.get(queue[i]) + 1); queue.push(edge.to); }
  const grouped = new Map();
  for (let s = 0; s < a.stateCount; s++) {
    const level = levels.get(s) ?? 0;
    if (!grouped.has(level)) grouped.set(level, []);
    grouped.get(level).push(s);
  }
  const maxRows = Math.max(...Array.from(grouped.values(), g => g.length));
  const width = Math.max(380, (Math.max(...grouped.keys()) + 1) * 130 + 80), height = Math.max(220, maxRows * 95 + 110);
  const positions = new Map();
  for (const [level, states] of grouped) states.forEach((s, i) => positions.set(s, [65 + level * 130, 75 + i * 95]));
  const svg = svgElement('svg', { viewBox: '0 0 ' + width + ' ' + height, role: 'img', 'aria-label': kind + ' state diagram' });
  svg.style.minWidth = width + 'px';
  svg.style.maxHeight = '650px';
  const defs = svgElement('defs', {}, svg), marker = svgElement('marker', { id: 'arrow-' + kind, viewBox: '0 0 10 10', refX: 9, refY: 5, markerWidth: 7, markerHeight: 7, orient: 'auto' }, defs);
  svgElement('path', { d: 'M0 0L10 5L0 10z', fill: '#7b8a94' }, marker);
  const combined = new Map();
  for (const e of a.transitions) {
    const key = e.from + ':' + e.to;
    if (!combined.has(key)) combined.set(key, { from: e.from, to: e.to, symbols: [] });
    combined.get(key).symbols.push(e.symbol);
  }
  for (const e of combined.values()) {
    const [x, y] = positions.get(e.from), [tx, ty] = positions.get(e.to);
    let d, lx, ly;
    if (e.from === e.to) { d = 'M' + (x - 13) + ' ' + (y - 17) + 'C' + (x - 45) + ' ' + (y - 65) + ' ' + (x + 45) + ' ' + (y - 65) + ' ' + (x + 13) + ' ' + (y - 17); lx = x; ly = y - 53; }
    else {
      const dx = tx - x, dy = ty - y, len = Math.hypot(dx, dy);
      const sx = x + dx / len * 23, sy = y + dy / len * 23, ex = tx - dx / len * 27, ey = ty - dy / len * 27;
      const cx = (x + tx) / 2 + (dy ? 18 : 0), cy = (y + ty) / 2 + (tx <= x ? 45 : -20);
      d = 'M' + sx + ' ' + sy + 'Q' + cx + ' ' + cy + ' ' + ex + ' ' + ey; lx = (sx + 2 * cx + ex) / 4; ly = (sy + 2 * cy + ey) / 4 - 5;
    }
    svgElement('path', { d, fill: 'none', stroke: '#7b8a94', 'stroke-width': 1.7, 'stroke-dasharray': e.symbols.every(s => s === null) ? '5 4' : '', 'marker-end': 'url(#arrow-' + kind + ')' }, svg);
    svgElement('text', { x: lx, y: ly, 'text-anchor': 'middle', 'font-size': 12, class: 'halo' }, svg).textContent = e.symbols.map(displaySymbol).join(', ');
  }
  const [sx, sy] = positions.get(a.start);
  svgElement('path', { d: 'M' + (sx - 50) + ' ' + sy + 'L' + (sx - 27) + ' ' + sy, stroke: '#0f6e7a', 'stroke-width': 2, 'marker-end': 'url(#arrow-' + kind + ')' }, svg);
  for (const [s, [x, y]] of positions) {
    const group = svgElement('g', { 'data-state': s }, svg);
    svgElement('circle', { cx: x, cy: y, r: 23, fill: s === a.start ? '#d9ecee' : '#fbfcfb', stroke: '#14232f', 'stroke-width': 2 }, group);
    if (a.accept.includes(s)) svgElement('circle', { cx: x, cy: y, r: 19, fill: 'none', stroke: '#14232f' }, group);
    svgElement('text', { x, y: y + 4, 'text-anchor': 'middle', 'font-size': 12 }, group).textContent = (kind === 'nfa' ? 'q' : 'D') + s;
    svgElement('title', {}, group).textContent = 'State ' + s + (a.sets ? ': {' + a.sets[s].join(', ') + '}' : '');
  }
  wrap.append(svg); return wrap;
}
function renderAutomaton(a, kind) {
  const container = $(kind + '-content'); container.replaceChildren(drawAutomaton(a, kind));
  const details = element('details'); details.open = a.stateCount > 40;
  details.append(element('summary', 'Transitions and state sets'));
  const label = element('label', 'Filter by state ID (blank shows all)'), filter = element('input'); filter.type = 'text'; filter.placeholder = 'e.g. 0'; filter.className = 'state-filter'; label.append(filter);
  const results = element('div'); details.append(label, results);
  const render = () => {
    const query = filter.value.trim(), state = query === '' ? null : Number(query);
    const edges = a.transitions.filter(e => state === null || e.from === state || e.to === state);
    results.replaceChildren(table(['From', 'Symbol', 'To'], edges.slice(0, 500).map(e => [e.from, displaySymbol(e.symbol), e.to]), 'Showing ' + Math.min(edges.length, 500) + ' of ' + edges.length + ' matching transitions'));
    if (a.sets) results.append(table(['State', a.provenance], a.sets.map((s, i) => [i, '{' + s.join(', ') + '}']).filter(row => state === null || row[0] === state)));
  };
  filter.addEventListener('input', render); render(); container.append(details);
}
function renderPositions(c) {
  $('position-content').replaceChildren(table(['Node (postorder)', 'Nullable', 'Firstpos', 'Lastpos'], c.positionFacts.map(f => [f.node, f.nullable, '{' + f.firstpos.join(', ') + '}', '{' + f.lastpos.join(', ') + '}'])),
    table(['Position', 'Symbol', 'Followpos'], c.positions.map(p => [p.position, p.symbol === null ? '# (end)' : displaySymbol(p.symbol), '{' + p.followpos.join(', ') + '}'])));
}
function renderTrace(c) {
  const m = c.match;
  $('test-result').textContent = (m.accepted ? 'ACCEPT' : 'REJECT') + ' · ' + (m.input === '' ? 'empty string ε' : JSON.stringify(m.input));
  $('test-result').className = m.accepted ? 'accepted' : 'rejected';
  const rows = m.dfaStates.map((s, i) => [i, i === 0 ? 'start' : displaySymbol(m.input[i - 1]), s === -1 ? 'dead' : s, m.minimizedStates[i], '{' + m.nfaSets[i].join(', ') + '}']);
  // Very long inputs remain inspectable via the slider without thousands of DOM rows.
  $('trace-content').replaceChildren(table(['Step', 'Consumed', 'DFA', 'Min DFA', 'ε-NFA closure'], rows.slice(0, 300), 'First ' + Math.min(rows.length, 300) + ' of ' + rows.length + ' steps; slider covers all steps'));
  $('trace-step').max = m.input.length; $('trace-step').value = 0; $('trace-step').disabled = false;
  updateTrace(0);
}
function updateTrace(step) {
  if (!current) return;
  const m = current.match;
  $('trace-label').textContent = 'Step ' + step + '/' + m.input.length + ': consumed ' + JSON.stringify(m.input.slice(0, step)) + '; DFA ' + (m.dfaStates[step] === -1 ? 'dead' : m.dfaStates[step]) + '; NFA {' + m.nfaSets[step].join(', ') + '}';
  for (const kind of ['nfa', 'dfa', 'minimized', 'direct']) {
    const active = kind === 'nfa' ? m.nfaSets[step] : [kind === 'dfa' ? m.dfaStates[step] : kind === 'minimized' ? m.minimizedStates[step] : m.directStates[step]];
    $(kind + '-content').querySelectorAll('g[data-state]').forEach(g => g.classList.toggle('trace-active', active.includes(Number(g.dataset.state))));
  }
}
function renderCompilation(c) {
  current = c;
  renderTokens(c.tokens); renderAST(c.ast);
  for (const kind of ['nfa', 'dfa', 'minimized', 'direct']) renderAutomaton(c[kind], kind);
  renderPositions(c);
  const stats = [['Tokens', c.tokens.length - 1], ['AST nodes', c.astNodes], ['NFA states / edges', c.nfa.stateCount + ' / ' + c.nfa.transitions.length], ['DFA states', c.dfa.stateCount], ['Minimized states', c.minimized.stateCount], ['Direct states', c.direct.stateCount]];
  $('stats').replaceChildren(...stats.map(([name, value]) => { const row = element('div'); row.append(element('dt', name), element('dd', value)); return row; }));
  $('agreement').textContent = '✓ Both constructions and their minimized DFAs are language-equivalent.';
  renderTrace(c);
}
function invalidate() {
  generation++;
  current = null;
  for (const kind of ['ast', 'nfa', 'dfa', 'minimized', 'direct', 'position', 'trace']) $(kind + '-content').replaceChildren();
  for (const id of ['token-list', 'stats', 'comparison', 'optimization']) $(id).replaceChildren();
  for (const id of ['agreement', 'test-result', 'trace-label', 'equivalence']) $(id).textContent = '';
  $('trace-step').disabled = true;
  refreshActions();
}
function compile() {
  if (busy) return;
  invalidate();
  const version = generation, regex = $('regex').value, input = $('test-input').value;
  return perform(async () => {
    const c = await api('compile', { regex, input });
    if (version === generation && regex === $('regex').value) renderCompilation(c);
  });
}
const mean = values => values.reduce((a, b) => a + b, 0) / values.length;
function metrics(c) {
  return ['thompson_subset', 'direct'].map(method => {
    const samples = c.samples.filter(s => s.method === method), shape = method === 'direct' ? c.direct : c.subset;
    const build = samples.map(s => s.constructionUs), minimize = samples.map(s => s.minimizationUs), avg = mean(build);
    const sd = Math.sqrt(mean(build.map(v => (v - avg) ** 2)));
    return [method === 'direct' ? 'Direct position DFA' : 'Thompson + subset', method === 'direct' ? '—' : c.nfaStates + ' / ' + c.nfaTransitions,
      shape.states + ' / ' + shape.transitions, shape.minStates + ' / ' + shape.minTransitions,
      ((shape.states - shape.minStates) / shape.states * 100).toFixed(1) + '%', avg.toFixed(2) + ' ± ' + sd.toFixed(2), mean(minimize).toFixed(2)];
  });
}
$('compile').addEventListener('click', compile);
$('regex').addEventListener('keydown', e => { if (e.key === 'Enter' && !busy) compile(); });
$('regex').addEventListener('input', () => { invalidate(); $('status').textContent = 'Expression changed. Compile to inspect it.'; });
$('test').addEventListener('click', () => {
  const version = generation, regex = current.regex;
  perform(async () => { const c = await api('compile', { regex, input: $('test-input').value }); if (version === generation) renderCompilation(c); });
});
$('test-input').addEventListener('keydown', e => { if (e.key === 'Enter' && !$('test').disabled) $('test').click(); });
$('trace-step').addEventListener('input', () => updateTrace(Number($('trace-step').value)));
$('compare').addEventListener('click', () => {
  const version = generation, regex = current.regex;
  perform(async () => {
    const c = await api('compare', { regex });
    if (version !== generation) return;
    $('comparison').replaceChildren(table(['Method', 'NFA states / edges', 'DFA states / edges', 'Min states / edges', 'State reduction', 'Build mean ± SD (µs)', 'Min mean (µs)'], metrics(c), c.repetitions + ' runs per method; language equivalence: ' + c.equivalent),
      element('p', 'Complete minimized DFAs include reachable sinks; their state count can exceed a partial DFA. Timings exclude parsing, JSON, startup, and equivalence checking.', 'side-note'));
  });
});
$('optimize').addEventListener('click', () => {
  const version = generation, regex = current.regex;
  perform(async () => {
    const result = await api('optimize', { regex });
    if (version !== generation) return;
    const content = $('optimization'); content.replaceChildren(element('p', 'Result: ' + (result.optimized === '' ? 'ε' : result.optimized), 'optimized'));
    if (!result.rewrites.length) content.append(element('p', 'No reducing rule applies.'));
    else content.append(table(['Rule', 'Before', 'After', 'AST nodes', 'Proof (visited pairs)'], result.rewrites.map(r => [r.rule, r.before, r.after, r.beforeNodes + ' → ' + r.afterNodes, 'Equivalent (' + r.proof.visitedPairs + ')'])));
    const apply = element('button', 'Compile result'); apply.type = 'button';
    apply.addEventListener('click', () => { $('regex').value = result.optimized; compile(); }); content.append(apply);
  });
});
$('equivalent').addEventListener('click', () => {
  const version = generation, regex = current.regex;
  perform(async () => {
    const result = await api('equivalent', { regex, candidate: $('candidate').value });
    if (version !== generation) return;
    $('equivalence').textContent = result.equivalent ? 'Equivalent languages. Explored ' + result.visitedPairs + ' state pairs.' : 'Different languages. Shortest counterexample: ' + (result.counterexample === '' ? 'ε (empty string)' : JSON.stringify(result.counterexample));
  });
});
$('benchmarks').addEventListener('click', () => perform(async () => {
  const response = await fetch('/api/benchmarks'), data = await response.json();
  if (!response.ok) throw new Error(data.error);
  $('benchmark-content').replaceChildren(element('p', 'Recorded ' + new Date(data.recordedAt).toLocaleString() + ' · ' + data.environment.cpu + ' · ' + data.environment.compiler, 'side-note'),
    table(['Regex', 'NFA', 'Subset DFA', 'Direct DFA', 'Min DFA', 'Subset build µs', 'Direct build µs', 'Equivalent'], data.cases.map(c => {
      const rows = metrics(c);
      return [c.regex, c.nfaStates, c.subset.states, c.direct.states, c.subset.minStates, rows[0][5], rows[1][5], c.equivalent];
    })), element('p', data.methodology, 'side-note'));
}));
const tabs = Array.from(document.querySelectorAll('[role="tab"]'));
function selectTab(tab) {
  tabs.forEach(t => { const selected = t === tab; t.setAttribute('aria-selected', selected); t.tabIndex = selected ? 0 : -1; $(t.getAttribute('aria-controls')).hidden = !selected; });
}
tabs.forEach((tab, i) => {
  tab.addEventListener('click', () => selectTab(tab));
  tab.addEventListener('keydown', e => {
    let next;
    if (e.key === 'ArrowRight') next = tabs[(i + 1) % tabs.length];
    if (e.key === 'ArrowLeft') next = tabs[(i - 1 + tabs.length) % tabs.length];
    if (e.key === 'Home') next = tabs[0];
    if (e.key === 'End') next = tabs[tabs.length - 1];
    if (next) { next.focus(); selectTab(next); e.preventDefault(); }
  });
});
compile();
