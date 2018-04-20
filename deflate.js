#!/usr/bin/env node
'use strict';
// Run DEFLATE algorithm on stdin, writes output to stdout
let deflate = require('zlib').createDeflateRaw({ level: 9 });
process.stdin.pipe(deflate);
deflate.pipe(process.stdout);
