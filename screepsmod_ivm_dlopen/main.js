'use strict';
const ivm = require('isolated-vm');
module.exports = function(config) {
	if (config.engine) {
		config.engine.on('playerSandbox', function(sandbox, userId) {
			let context = sandbox.getContext();

			let a = new ivm.Reference(function(path) {
				throw new Error(1);
				return (new ivm.NativeModule(path)).createSync(sandbox.getContext());
			});
			console.log(1,context.global, a)
			context.global.setSync('ivm_dlopen', new ivm.ExternalCopy(1));
			console.log(2,context.global, a)
		});
	}
}
