#!/usr/bin/env node
'use strict';
// Given a list of files it will create a tar-like archive. Screeps file sync is awful and one big
// file tends to work much better than several small files. This also handles files in directories
// for us too.
const fs = require('fs');
const path = require('path');
let ws = fs.createWriteStream(process.argv[2]);
let relative;
for (let ii = 3; ii < process.argv.length; ++ii) {
	let arg = process.argv[ii];
	if (arg == '--relative') {
		relative = process.argv[++ii];
	} else {
		fs.readFile(arg, function(relative, err, data) {
			if (err) {
				throw err;
			}
			if (arg.substr(0, relative.length) !== relative) {
				throw new Error(`Path not relative: ${arg} to ${relative}`);
			}
			arg = arg.substr(relative.length + 1);
			let header = new Uint8Array(arg.length + 8);
			let headerView = new DataView(header.buffer);
			headerView.setUint32(0, arg.length);
			for (let ii = 0; ii < arg.length; ++ii) {
				header[ii + 4] = arg.charCodeAt(ii);
			}
			headerView.setUint32(arg.length + 4, data.length);
			ws.write(header);
			ws.write(data);
		}.bind(null, relative));
	}
}

// Delete output on error
process.on('uncaughtException', function(err) {
	fs.unlinkSync(process.argv[2]);
	throw err;
});
