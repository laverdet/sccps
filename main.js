'use strict';
// const the_general = require('the_general');
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
console.error = console.log;
const mod = runtime({
	ENVIRONMENT: 'SHELL',
	// wasmBinary: the_general,
	print: line => console.log(line),
	printErr: line => console.error(line),
	noInitialRun: true,
	noExitRuntime: true,
	screeps: screeps,
});

try {
	mod.init();
} catch (err) {
	console.log('Uncaught error in init()', err.stack || err.message || err);
	throw err;
}
module.exports.loop = function() {
	try {
		mod.preloop();
		mod.loop();
	} catch (err) {
		console.log('Uncaught error', err.stack || err.message || err);
		throw err;
	}
};
