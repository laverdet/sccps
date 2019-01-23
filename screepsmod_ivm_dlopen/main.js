'use strict';
const fs = require('fs');
const ivm = require('isolated-vm');
module.exports = function(config) {
	// Track unclean exits of runner
	let foundRunnerMarker = false;
	try {
		fs.statSync('.runner.marker');
		foundRunnerMarker = true;
	} catch (err) {}

	// Set up dlopen method
	if (config.engine) {
		let env = config.common.storage.env;
		config.engine.on('init', function(type) {
			if (type === 'main') {
				if (foundRunnerMarker) {
					env.get(env.keys.MAIN_LOOP_PAUSED).then(function(val) {
						if (val === '0') {
							console.log('Previous runner exitted uncleanly. Pausing loop.');
						}
					});
					env.set(env.keys.MAIN_LOOP_PAUSED, '1');
				}
			} else if (type === 'runner') {
				fs.writeFileSync('.runner.marker', '');
				let cleanup = function() {
					try {
						fs.unlinkSync('.runner.marker');
					} catch (err) {}
				}
				process.on('exit', cleanup);
				process.on('SIGINT', () => { cleanup(); process.exit(0); });
			}
		});
		config.engine.on('playerSandbox', function(sandbox, userId) {
			let context = sandbox.getContext();
			context.global.setSync('ivm_dlopen', new ivm.Reference(function(path) {
				let module = new ivm.NativeModule(path);
				let instance = module.createSync(sandbox.getContext());
				instance.module = module;
				return instance;
			}));
		});
	}
}
