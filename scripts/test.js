const path = require('node:path');
const { spawnSync } = require('node:child_process');
const root = path.resolve(__dirname, '..');
for (const [command, args] of [
  [path.join(root, 'build', process.platform === 'win32' ? 'compiler_tests.exe' : 'compiler_tests'), []],
  [process.execPath, ['--test', 'tests/server.test.js']]
]) {
  const result = spawnSync(command, args, { cwd: root, stdio: 'inherit' });
  if (result.error) throw result.error;
  if (result.status !== 0) process.exit(result.status || 1);
}
