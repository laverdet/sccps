#!/usr/bin/env node
'use strict';
// Given a list of files it will create a tar-like archive. Screeps file sync is awful and one big
// file tends to work much better than several small files. This also handles files in directories
// for us too.
const fs = require('fs');
const path = require('path');
let relative = process.argv[2];
let ws = fs.createWriteStream(process.argv[3]);
process.argv.slice(4).forEach(function(name) {
	fs.readFile(name, function(err, data) {
		if (err) {
			throw err;
		}
		if (name.substr(0, relative.length) !== relative) {
			throw new Error(`Path not relative: ${name}`);
		}
		name = name.substr(relative.length + 1);
		let header = new Uint8Array(name.length + 8);
		let headerView = new DataView(header.buffer);
		headerView.setUint32(0, name.length);
		for (let ii = 0; ii < name.length; ++ii) {
			header[ii + 4] = name.charCodeAt(ii);
		}
		headerView.setUint32(name.length + 4, data.length);
		ws.write(header);
		ws.write(data);
	});
});
