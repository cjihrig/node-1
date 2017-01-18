'use strict';
// Flags: --expose_internals
const cluster = require('cluster');
const http = require('http');

if (cluster.isMaster) {
  const Scheduler = require('internal/cluster/round_robin_handle');

  Scheduler.prototype.distribute = function(err, handle) {
    console.log(this.free);
    this.handles.push(handle);
    this.handoff(this.free[0]);
  };

  cluster.schedulingPolicy = cluster.SCHED_CUSTOM;
  cluster.setupMaster({ scheduler: Scheduler });

  const w1 = cluster.fork();
  const w2 = cluster.fork();
} else {
  http.createServer((req, res) => {
    res.end(`hello from ${cluster.worker.id}\n`);
  }).listen(4000);
}
