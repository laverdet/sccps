#!/usr/bin/env node
'use strict';
let deflate = require('zlib').createDeflateRaw({ level: 9 });
process.stdin.pipe(deflate);
deflate.pipe(process.stdout);
