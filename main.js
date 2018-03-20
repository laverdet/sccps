'use strict';
function error(line) {
	console.log(`<span style="color:#e79da7">${line}</span>`);
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
		try {
			mod.loop();
		} catch (err) {
			console.error('Uncaught error', err.stack || err.message || err);
			throw err;
		}
	}
};
