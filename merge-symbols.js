#!/usr/bin/env node
'use strict';
// Reads list of symbols from each object file and aggregates the undefined symbols to be linked
// into the main module. Debug builds only.
const fs = require('fs');
const path = require('path');

function readSymbols(file) {
	let def = Object.create(null);
	let undef = Object.create(null);
	try {
		let nm = fs.readFileSync(file, 'utf8');
		nm.split(/\n/g).map(function(line) {
			let match = /^[0-9a-f \-]+([UCWtTdD\?]) (.+)$/.exec(line);
			if (match) {
				(match[1] === 'U' ? undef : def)[`_${match[2]}`] = true;
			}
		});
	} catch (err) {}
	return { def, undef };
}

let outputFile = process.argv[2];
let token = process.argv.indexOf('--');
let mainObjects = process.argv.slice(3, token);
let dylibs = process.argv.slice(token + 1);

let def = Object.create(null);
let undef = Object.create(null);
for (let file of mainObjects) {
	for (let symbol in readSymbols(file).def) {
		def[symbol] = undef[symbol] = true;
	}
}

// We add all currently defined main symbols to undefined as well so that if symbols come and go
// during debugging we won't have to rebuild the main module over and over.
let hasExistingDatabase = true;
try {
	for (let symbol of JSON.parse(fs.readFileSync(outputFile, 'utf8'))) {
		def[symbol] = undef[symbol] = true;
	}
} catch (err) {
	hasExistingDatabase = false;
}

let dylibSymbols = dylibs.map(readSymbols);
for (let file in dylibSymbols) {
	for (let symbol in dylibSymbols[file].def) {
		def[symbol] = true;
	}
}

let didUpdate = false;
for (let file in dylibSymbols) {
	for (let symbol in dylibSymbols[file].undef) {
		if (!def[symbol]) {
			if (hasExistingDatabase && !undef[symbol]) {
				console.log(`\x1b[33mDylib requires new symbol: ${symbol}`);
			}
			didUpdate = true;
			undef[symbol] = true;
		}
	}
}

if (didUpdate) {
	fs.writeFileSync(outputFile, JSON.stringify(Object.keys(undef)));
	if (hasExistingDatabase) {
		console.log('\n\x1b[31mMain module must be rebuilt. Please re-run `make`.\n');
		process.exit(1);
	}
}
