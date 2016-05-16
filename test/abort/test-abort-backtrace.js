'use strict';
require('../common');
const assert = require('assert');
const cp = require('child_process');

if (process.argv[2] === 'child') {
  process.abort();
} else {
  const child = cp.spawnSync(`${process.execPath}`,
                             [`${__filename}`, 'child']);
  const frames = child.stderr.toString().trim().split('\n');

  assert.strictEqual(child.stdout.toString().trim(), '');
  assert.ok(frames.length > 0);
  assert.ok(frames.every((frame, i) => {
    const re = new RegExp(`\w*${i + 1}: .+( \[.+\])?`);

    return re.test(frame);
  }));
}
