// TODO(cjihrig): Put WASI behind a flag.
// TODO(cjihrig): Provide a mechanism to bind to WASM modules.
'use strict';
const { Array, ArrayPrototype } = primordials;
const { ERR_INVALID_ARG_TYPE } = require('internal/errors').codes;
const { WASI: _WASI } = internalBinding('wasi');


class WASI {
  constructor(options = {}) {
    if (options === null || typeof options !== 'object')
      throw new ERR_INVALID_ARG_TYPE('options', 'object', options);

    let { args, env, preopens } = options;

    if (Array.isArray(args))
      args = ArrayPrototype.map(args, (arg) => { return String(arg); });
    else if (args === undefined)
      args = [];
    else
      throw new ERR_INVALID_ARG_TYPE('options.args', 'Array', args);

    const envPairs = [];

    if (env !== null && typeof env === 'object') {
      for (const key in env) {
        const value = env[key];
        if (value !== undefined)
          envPairs.push(`${key}=${value}`);
      }
    } else if (env !== undefined) {
      throw new ERR_INVALID_ARG_TYPE('options.env', 'Object', env);
    }

    if (preopens == null) {
      preopens = null;
    } else if (typeof preopens !== 'object') {
      throw new ERR_INVALID_ARG_TYPE('options.preopens', 'Object', preopens);
    } else {
      //
    }

    // TODO(cjihrig): Validate preopen object schema.

    const memory = Buffer.allocUnsafe(200000);
    const view = new DataView(memory.buffer);
    const wasi = new _WASI(args, envPairs, preopens, memory);

    const argc = 0;
    const argvBufSize = 4;
    const environCount = 8;
    const environBufSize = 12;
    const environ = 16;
    let r;

    r = wasi.args_sizes_get(argc, argvBufSize);
    console.log('r', r);
    console.log('argc', view.getUint32(argc, true));
    console.log('argvBufSize', view.getUint32(argvBufSize, true));

    r = wasi.environ_sizes_get(environCount, environBufSize);
    console.log('r', r);
    console.log('environCount', view.getUint32(environCount, true));
    console.log('environBufSize', view.getUint32(environBufSize, true));

    const environBuf = 20 + (view.getUint32(environCount, true) * 8);
    console.log('environ', environ, 'environBuf', environBuf);
    r = wasi.environ_get(environ, environBuf);
    console.log('r', r);

    console.log('memory', memory);
  }
}


module.exports = { WASI };
