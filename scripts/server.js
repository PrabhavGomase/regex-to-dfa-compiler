const http = require('node:http');
const fs = require('node:fs');
const path = require('node:path');
const { spawn } = require('node:child_process');
const root = path.resolve(__dirname, '..');
const executable = path.join(root, 'build', process.platform === 'win32' ? 'regexlab.exe' : 'regexlab');
const operations = new Set(['compile', 'compare', 'optimize', 'equivalent']);
const assets = new Map([['/', ['frontend/index.html', 'text/html']], ['/index.html', ['frontend/index.html', 'text/html']],
  ['/script.js', ['frontend/script.js', 'text/javascript']], ['/style.css', ['frontend/style.css', 'text/css']],
  ['/api/benchmarks', ['docs/benchmark-results.json', 'application/json']]]);
function reply(res, status, body, type = 'application/json') {
  if (res.destroyed || res.writableEnded) return;
  res.writeHead(status, { 'Content-Type': type + '; charset=utf-8', 'Cache-Control': 'no-store',
    'X-Content-Type-Options': 'nosniff', 'Content-Security-Policy': "default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; object-src 'none'; frame-ancestors 'none'" });
  res.end(typeof body === 'string' ? body : JSON.stringify(body));
}
function createServer() {
  let active = 0;
  return http.createServer((req, res) => {
    // Loopback-only server: reject arbitrary Host values and cross-origin requests.
    const address = req.headers.host || '';
    if (!/^(localhost|127\.0\.0\.1):\d+$/.test(address) ||
        (req.headers.origin && req.headers.origin !== 'http://' + address))
      return reply(res, 403, { error: 'Use the local RegexLab page to make requests' });
    const url = new URL(req.url, 'http://' + address);
    if (req.method === 'GET' && assets.has(url.pathname)) {
      const [file, type] = assets.get(url.pathname);
      return fs.readFile(path.join(root, file), (error, body) => {
        if (error) return reply(res, 404, { error: file.includes('benchmark') ? 'Run node scripts/benchmark.js to record benchmarks' : 'File unavailable' });
        reply(res, 200, body.toString('utf8'), type);
      });
    }
    const operation = url.pathname.replace('/api/', '');
    if (req.method !== 'POST' || !url.pathname.startsWith('/api/') || !operations.has(operation))
      return reply(res, 404, { error: 'Unknown endpoint' });
    if (!(req.headers['content-type'] || '').startsWith('application/json'))
      return reply(res, 415, { error: 'Expected application/json' });
    let body = '', size = 0, oversized = false;
    req.on('error', () => reply(res, 400, { error: 'Request interrupted' }));
    req.on('data', chunk => {
      size += chunk.length;
      if (size > 16384) { oversized = true; reply(res, 413, { error: 'Request exceeds 16 KiB' }); }
      else body += chunk.toString('utf8');
    });
    req.on('end', () => {
      if (oversized) return;
      let data;
      try { data = JSON.parse(body); } catch { return reply(res, 400, { error: 'Invalid JSON body' }); }
      if (!data || typeof data !== 'object' || Array.isArray(data)) return reply(res, 400, { error: 'Expected an object' });
      const regex = data.regex, input = data.input === undefined ? '' : data.input, candidate = data.candidate === undefined ? '' : data.candidate;
      for (const [name, value, cap] of [['regex', regex, 256], ['input', input, 4096], ['candidate', candidate, 256]]) {
        if (typeof value !== 'string' || value.length > cap || !/^[\x20-\x7e]*$/.test(value))
          return reply(res, 400, { error: name + ' must be printable ASCII, at most ' + cap + ' characters' });
      }
      if (active >= 2) return reply(res, 429, { error: 'Compiler is busy; try again shortly' });
      active++;
      // No shell, no user-controlled command arguments; request values travel over stdin.
      const child = spawn(executable, ['--request'], { cwd: root, windowsHide: true, stdio: ['pipe', 'pipe', 'pipe'] });
      let output = '', stderr = '', finished = false;
      const finish = (status, result) => {
        if (finished) return;
        finished = true;
        active--;
        clearTimeout(timer);
        reply(res, status, result);
      };
      const timer = setTimeout(() => { child.kill(); finish(408, { error: 'Compiler request timed out after 15 seconds' }); }, 15000);
      child.on('error', () => finish(503, { error: 'Compiler unavailable. Run node scripts/build.js first' }));
      child.stdin.on('error', () => {});
      child.stdout.on('data', chunk => {
        output += chunk.toString('utf8');
        if (output.length > 16 * 1024 * 1024) { child.kill(); finish(413, { error: 'Compiler output exceeds 16 MiB' }); }
      });
      child.stderr.on('data', chunk => { if (stderr.length < 4096) stderr += chunk; });
      child.on('close', code => {
        if (finished) return;
        try {
          const result = JSON.parse(output);
          finish(code === 0 ? 200 : 400, result);
        } catch { finish(500, { error: 'Compiler returned invalid output' + (stderr ? ': ' + stderr.trim() : '') }); }
      });
      res.on('close', () => { if (!finished) { child.kill(); finish(499, { error: 'Request closed' }); } });
      child.stdin.end([operation, regex, input, candidate, ''].join('\n'));
    });
  });
}
if (require.main === module) {
  const port = Number(process.env.PORT || 3000);
  const server = createServer();
  server.on('error', e => { console.error(e.message); process.exitCode = 1; });
  server.listen(port, '127.0.0.1', () => console.log('RegexLab: http://127.0.0.1:' + server.address().port));
}
module.exports = { createServer };
