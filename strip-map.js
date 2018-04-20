#!/usr/bin/env node
'use strict';
// Reads a source map from stdin and strips out source code information, leaving only line number
// information. Also normalizes path names.
const buffer = require('buffer');
const fs = require('fs');
const path = require('path');
let buffers = [];
process.stdin.resume();
process.stdin.on('data', buffer => buffers.push(buffer));
process.stdin.on('end', function() {
	let map = JSON.parse(Buffer.concat(buffers).toString('utf8'));
	delete map.sourcesContent;
	map.sources = map.sources.map(function(file) {
		return path.normalize(file.replace(/^.+\/system\/include\//, ''));
	});
	process.stdout.write(JSON.stringify(map));
});
