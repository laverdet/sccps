#!/usr/bin/env node
'use strict';
// Given a list of JS files it will read each one plus a corresponding source map file, outputting
// to stdout like this: `{'module': 'map': {}, 'src': '...'}`
const fs = require('fs');
const path = require('path');
let dylibs = process.argv.slice(2);
process.stdout.write(JSON.stringify(Object.keys(dylibs).reduce(function(output, ii) {
	let file = dylibs[ii];
	let map = JSON.parse(fs.readFileSync(`${file}.map`, 'utf8'));
	let src = fs.readFileSync(file, 'utf8');
	delete map.sourcesContent;
	map.sources = map.sources.map(function(file) {
		return path.normalize(file.replace(/^.+\/system\/include\//, ''));
	});
	output[path.basename(file).replace(/\.js$/, '')] = { map, src };
	return output;
}, {})));
