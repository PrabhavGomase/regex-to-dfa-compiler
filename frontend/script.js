// Prototype data: saved compiler output for (a|b)*abb.
// Replace this with real output from the C++ compiler once it is connected.

const SAMPLE_REGEX = "(a|b)*abb";

const TOKENS = [
  { type: "LPAREN" },
  { type: "LITERAL", value: "a" },
  { type: "OR" },
  { type: "LITERAL", value: "b" },
  { type: "RPAREN" },
  { type: "STAR" },
  { type: "LITERAL", value: "a" },
  { type: "LITERAL", value: "b" },
  { type: "LITERAL", value: "b" },
  { type: "END" },
];

const TOKEN_SYMBOLS = {
  LPAREN: "(", OR: "|", RPAREN: ")", STAR: "*", PLUS: "+", QUESTION: "?", END: "end",
};

// AST nodes with hand-placed coordinates (viewBox 640 x 420)
const AST_NODES = {
  n0: { label: "CONCAT", x: 420, y: 30 },
  n1: { label: "CONCAT", x: 300, y: 100 },
  n2: { label: "b", x: 540, y: 100, leaf: true },
  n3: { label: "CONCAT", x: 200, y: 170 },
  n4: { label: "b", x: 400, y: 170, leaf: true },
  n5: { label: "STAR", x: 120, y: 240 },
  n6: { label: "a", x: 280, y: 240, leaf: true },
  n7: { label: "UNION", x: 120, y: 310 },
  n8: { label: "a", x: 60, y: 380, leaf: true },
  n9: { label: "b", x: 180, y: 380, leaf: true },
};

const AST_EDGES = [
  ["n0", "n1"], ["n0", "n2"], ["n1", "n3"], ["n1", "n4"],
  ["n3", "n5"], ["n3", "n6"], ["n5", "n7"], ["n7", "n8"], ["n7", "n9"],
];

// NFA states with hand-placed coordinates (viewBox 1000 x 330)
const NFA_POS = {
  q6: [40, 150], q4: [120, 150], q0: [205, 70], q2: [205, 230],
  q1: [290, 70], q3: [290, 230], q5: [375, 150], q7: [455, 150],
  q8: [540, 150], q9: [620, 150], q10: [700, 150], q11: [780, 150],
  q12: [860, 150], q13: [940, 150],
};

const NFA_START = "q6";
const NFA_ACCEPT = "q13";

// [from, to, symbol (null = epsilon), optional curve control y]
const NFA_EDGES = [
  ["q0", "q1", "a"], ["q2", "q3", "b"],
  ["q4", "q0", null], ["q4", "q2", null],
  ["q1", "q5", null], ["q3", "q5", null],
  ["q6", "q4", null], ["q6", "q7", null, 420],
  ["q5", "q4", null, -120], ["q5", "q7", null],
  ["q7", "q8", null], ["q8", "q9", "a"],
  ["q9", "q10", null], ["q10", "q11", "b"],
  ["q11", "q12", null], ["q12", "q13", "b"],
];

const SVG_NS = "http://www.w3.org/2000/svg";
const R = 17;

function svgEl(name, attrs, parent) {
  const el = document.createElementNS(SVG_NS, name);
  for (const key in attrs) el.setAttribute(key, attrs[key]);
  if (parent) parent.appendChild(el);
  return el;
}

function addMarker(svg, id, color) {
  const defs = svgEl("defs", {}, svg);
  const marker = svgEl("marker", {
    id, viewBox: "0 0 10 10", refX: 9, refY: 5,
    markerWidth: 8, markerHeight: 8, orient: "auto-start-reverse",
  }, defs);
  svgEl("path", { d: "M0,0 L10,5 L0,10 z", fill: color }, marker);
}

// move from point p toward point q by distance d
function shift(p, q, d) {
  const dx = q[0] - p[0], dy = q[1] - p[1];
  const len = Math.hypot(dx, dy) || 1;
  return [p[0] + (dx / len) * d, p[1] + (dy / len) * d];
}

/* ---------- tokens ---------- */
function renderTokens() {
  const list = document.getElementById("token-list");
  list.innerHTML = "";
  TOKENS.forEach((tok) => {
    const li = document.createElement("li");
    li.className = "token" + (tok.type === "LITERAL" ? " literal" : "") + (tok.type === "END" ? " end" : "");
    const value = document.createElement("span");
    value.className = "tk-value";
    value.textContent = tok.type === "LITERAL" ? tok.value : TOKEN_SYMBOLS[tok.type];
    const type = document.createElement("span");
    type.className = "tk-type";
    type.textContent = tok.type;
    li.append(value, type);
    list.appendChild(li);
  });
}

/* ---------- AST ---------- */
function renderAST() {
  const svg = document.getElementById("ast-svg");
  svg.innerHTML = "";
  svg.setAttribute("viewBox", "0 0 640 420");

  AST_EDGES.forEach(([from, to]) => {
    const a = AST_NODES[from], b = AST_NODES[to];
    svgEl("line", {
      x1: a.x, y1: a.y + 14, x2: b.x, y2: b.y - 14,
      stroke: "#14232f", "stroke-width": 1.5,
    }, svg);
  });

  Object.values(AST_NODES).forEach((n) => {
    const w = n.leaf ? 36 : 88;
    svgEl("rect", {
      x: n.x - w / 2, y: n.y - 14, width: w, height: 28, rx: 2,
      fill: n.leaf ? "#d9ecee" : "#fbfcfb", stroke: "#14232f", "stroke-width": 1.5,
    }, svg);
    const t = svgEl("text", {
      x: n.x, y: n.y + 5, "text-anchor": "middle", "font-size": 13,
      "font-weight": n.leaf ? 600 : 400,
    }, svg);
    t.textContent = n.label;
  });
}

/* ---------- NFA ---------- */
function renderNFA() {
  const svg = document.getElementById("nfa-svg");
  svg.innerHTML = "";
  svg.setAttribute("viewBox", "0 0 1000 330");
  addMarker(svg, "arrow-sym", "#14232f");
  addMarker(svg, "arrow-eps", "#7b8a94");

  // edges first so states draw on top
  NFA_EDGES.forEach(([from, to, symbol, curveY]) => {
    const p = NFA_POS[from], q = NFA_POS[to];
    const isEps = symbol === null;
    const stroke = isEps ? "#7b8a94" : "#14232f";
    const common = {
      fill: "none", stroke, "stroke-width": isEps ? 1.4 : 2,
      "marker-end": `url(#${isEps ? "arrow-eps" : "arrow-sym"})`,
    };
    if (isEps) common["stroke-dasharray"] = "5 4";

    let labelX, labelY;
    if (curveY !== undefined) {
      const c = [(p[0] + q[0]) / 2, curveY];
      const s = shift(p, c, R), e = shift(q, c, R);
      svgEl("path", { ...common, d: `M ${s[0]} ${s[1]} Q ${c[0]} ${c[1]} ${e[0]} ${e[1]}` }, svg);
      labelX = 0.25 * s[0] + 0.5 * c[0] + 0.25 * e[0];
      labelY = 0.25 * s[1] + 0.5 * c[1] + 0.25 * e[1] + (curveY < 150 ? -6 : 14);
    } else {
      const s = shift(p, q, R), e = shift(q, p, R);
      svgEl("line", { ...common, x1: s[0], y1: s[1], x2: e[0], y2: e[1] }, svg);
      labelX = (s[0] + e[0]) / 2;
      labelY = (s[1] + e[1]) / 2 - 7;
    }

    const t = svgEl("text", {
      x: labelX, y: labelY, "text-anchor": "middle", "font-size": 14,
      "font-weight": isEps ? 400 : 700, class: "halo",
    }, svg);
    t.textContent = isEps ? "ε" : symbol;
  });

  // start arrow
  const [sx, sy] = NFA_POS[NFA_START];
  svgEl("line", {
    x1: 4, y1: sy, x2: sx - R - 1, y2: sy,
    stroke: "#0f6e7a", "stroke-width": 2.5, "marker-end": "url(#arrow-sym)",
  }, svg);

  // states
  Object.entries(NFA_POS).forEach(([name, [x, y]]) => {
    if (name === NFA_ACCEPT) {
      svgEl("circle", { cx: x, cy: y, r: R + 4, fill: "none", stroke: "#14232f", "stroke-width": 1.5 }, svg);
    }
    svgEl("circle", {
      cx: x, cy: y, r: R,
      fill: name === NFA_START ? "#d9ecee" : "#fbfcfb",
      stroke: "#14232f", "stroke-width": 1.8,
    }, svg);
    const t = svgEl("text", { x, y: y + 4, "text-anchor": "middle", "font-size": 11 }, svg);
    t.textContent = name;
  });
}

/* ---------- summary ---------- */
function renderStats() {
  const stats = [
    ["Tokens", TOKENS.length - 1],
    ["AST nodes", Object.keys(AST_NODES).length],
    ["NFA states", Object.keys(NFA_POS).length],
    ["NFA transitions", NFA_EDGES.length],
    ["Start state", NFA_START],
    ["Accept state", NFA_ACCEPT],
  ];
  const dl = document.getElementById("stats");
  dl.innerHTML = "";
  stats.forEach(([label, value]) => {
    const row = document.createElement("div");
    const dt = document.createElement("dt");
    dt.textContent = label;
    const dd = document.createElement("dd");
    dd.textContent = value;
    row.append(dt, dd);
    dl.appendChild(row);
  });
}

/* ---------- tabs ---------- */
function selectTab(tab) {
  document.querySelectorAll('[role="tab"]').forEach((t) => {
    const selected = t === tab;
    t.setAttribute("aria-selected", selected);
    document.getElementById(t.getAttribute("aria-controls")).hidden = !selected;
  });
}

function setupTabs() {
  const tabs = Array.from(document.querySelectorAll('[role="tab"]'));
  tabs.forEach((tab, i) => {
    tab.addEventListener("click", () => selectTab(tab));
    tab.addEventListener("keydown", (e) => {
      let next = null;
      if (e.key === "ArrowRight") next = tabs[(i + 1) % tabs.length];
      if (e.key === "ArrowLeft") next = tabs[(i - 1 + tabs.length) % tabs.length];
      if (next) { next.focus(); selectTab(next); e.preventDefault(); }
    });
  });
}

/* ---------- compile button ---------- */
function compile() {
  const input = document.getElementById("regex").value.replace(/\s+/g, "");
  const status = document.getElementById("status");

  if (input === SAMPLE_REGEX) {
    status.textContent = "";
    renderTokens();
    renderAST();
    renderNFA();
    renderStats();
  } else {
    status.textContent =
      "This prototype only has saved output for (a|b)*abb. Live compiling needs the page connected to the C++ compiler.";
  }
}

document.getElementById("compile").addEventListener("click", compile);
document.getElementById("regex").addEventListener("keydown", (e) => {
  if (e.key === "Enter") compile();
});

setupTabs();
compile();