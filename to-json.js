#!/usr/bin/env node
'use strict';
// Turns stdin lines into a JSON array of strings
let buffers = [];
process.stdin.resume();
process.stdin.on('data', buffer => buffers.push(buffer));
process.stdin.on('end', function() {
	let lines = Buffer.concat(buffers).toString('utf8').split(/\s/g).filter(line => line);
	process.stdout.write(JSON.stringify(lines));
});
