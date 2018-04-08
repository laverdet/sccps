'use strict';
function error(...lines) {
	console.log(lines.map(line => `<span style="color:#e79da7">${line}</span>`));
}
module.exports.loop = function() {
	if (Game.cpu.limit + Game.cpu.bucket === Game.cpu.tickLimit) {
		error('Deferring WASM initialization due to empty bucket.');
		return;
	}
	console.error = error;
	const the_general = require('the_general');
	const runtime = require('runtime');
	const screeps = {
		array: require('array'),
		creep: require('creep'),
		game: require('game'),
		object: require('object'),
		position: require('position'),
		resource: require('resource'),
		room: require('room'),
		string: require('string'),
		structure: require('structure'),
		util: require('util'),
		vector: require('vector'),
	};
	const mod = runtime({
		ENVIRONMENT: 'SHELL',
		wasmBinary: the_general,
		print: line => console.log(line),
		printErr: line => console.log(`<span style="color:#e79da7">${line}</span>`),
		noInitialRun: true,
		noExitRuntime: true,
		screeps: screeps,
	});

	try {
		mod.__ZN7screeps12game_state_t4initEv();
	} catch (err) {
		error('Uncaught error in init()', err.stack || err.message || err);
		throw err;
	}
	module.exports.loop = function() {
		console.error = error; // Must be done every tick :(
		if (Game.cpu.limit + Game.cpu.bucket === Game.cpu.tickLimit) {
			error('Skipping loop due to empty bucket.');
			return;
		}
		try {
			mod.__Z4loopv();
		} catch (err) {
			if (typeof err === 'number') {
				let info = mod.lastException;
				let stack = info.stack;
				let err_info = mod.__ZN7screeps14exception_whatEPv(info.ptr);
				let [ name, what ] = [ mod.HEAPU32[err_info >> 2], mod.HEAPU32[(err_info + 4) >> 2] ].map(function(ptr) {
					let str = '';
					while (true) {
						let ch = mod.HEAP8[ptr++];
						if (!ch) {
							return str;
						}
						str += String.fromCharCode(ch);
					}
				});
				console.error(`${name}: ${what}\n${stack.split(/\n/g).slice(2).join('\n')}`);
				mod._free(info.ptr);
			} else {
				console.error('Uncaught error', err.stack || err.message || err);
			}
			throw err;
		}
	}
};
