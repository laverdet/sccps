'use strict';
module.exports = {
	setupExceptionHandler(mod, sourceMaps) {
		let demangleHole = mod._malloc(256 + 4 + 1);
		function readCString(ptr) {
			let str = '';
			while (true) {
				let ch = mod.HEAP8[ptr++];
				if (!ch) {
					return str;
				}
				str += String.fromCharCode(ch);
			}
		}

		function makeCString(string, ptr) {
			for (let ii = 0; ii < string.length; ++ii) {
				mod.HEAP8[ptr + ii] = string.charCodeAt(ii);
			}
			mod.HEAP8[ptr + string.length] = 0;
			return ptr;
		}

		function demangle(name) {
			if (!mod.___cxa_demangle) {
				return name;
			}
			let cName, status, ret;
			try {
				if (name.length > 256) {
					return name;
				}
				cName = makeCString(name.substr(1), demangleHole + 4);
				status = demangleHole;
				mod._malloc(4);
				ret = mod.___cxa_demangle(cName, 0, 0, status);
				if (mod.HEAP32[status >> 2] === 0 && ret) {
					return readCString(ret).replace(/std::__2::/g, 'std::');
				} else {
					return name;
				}
			} catch (err) {
				return name;
			} finally {
				try {
					ret && mod._free(ret);
				} catch (err) {}
			}
		}

		Error.stackTraceLimit = 25;
		Error.prepareStackTrace = function(error, stack) {
			// Parse message
			let message;
			if (error.ptr) {
				let err_info = mod.__ZN7screeps14exception_whatEPv(error.ptr);
				let [ name, what ] = [ mod.HEAPU32[err_info >> 2], mod.HEAPU32[(err_info + 4) >> 2] ].map(function(ptr) {
					let str = '';
					while (true) {
						let ch = mod.HEAPU8[ptr++];
						if (!ch) {
							return str;
						}
						str += String.fromCharCode(ch);
					}
				});
				mod._free(error.ptr);
				name = name.replace(/^St[0-9]+/, 'std::');
				message = `${name}: ${what}`;
			} else {
				message = error.message;
			}

			// Render frames
			let renderer;
			let lastFn;
			if (sourceMaps) {
				renderer = function(frame) {
					let fnName = frame.getFunctionName();
					let lastFnCopy = lastFn;
					lastFn = fnName;
					let demangled = demangle(fnName);
					let source;
					let line = frame.getLineNumber(); // Messy function bloat
					let column = frame.getColumnNumber();
					if (fnName !== demangled) {
						let libMatch = /eval at ([^ ]+)/.exec(frame.getEvalOrigin());
						if (libMatch !== null) {
							let sourceMap = sourceMaps[libMatch[1]];
							if (sourceMap !== undefined) {
								let position = sourceMap.originalPositionFor({ line, column });
								if (position.source !== null) {
									return `${demangled} (${position.source}:${position.line})`;
								} else if (fnName === lastFnCopy) {
									// Cross-library invocation will dupe the function
									return null;
								}
							}
						}
						let origin = frame.getEvalOrigin() || `${frame.getScriptNameOrSourceURL()}:${line}:${column}`;
						return `${demangled} (${origin})`;
					}
					return frame;
				};
			} else {
				renderer = function(frame) {
					let fnName = frame.getFunctionName();
					let demangled = demangle(fnName);
					if (fnName !== demangled) {
						let origin = frame.getEvalOrigin() || `${frame.getScriptNameOrSourceURL()}:${frame.getLineNumber()}:${frame.getColumnNumber()}`;
						return `${demangled} (${origin})`;
					}
					return frame;
				}
			}
			for (let ii = 0; ii < stack.length; ++ii) {
				if (stack[ii].getEvalOrigin() === '__mainLoop') {
					stack = stack.slice(0, ii);
					break;
				}
			}
			let noise = /^(?:dynCall|ftCall|invoke|_emscripten_asm_const)_[dfiv]+$|^asm\.|^_.+__wrapper$|^___cxa_throw/;
			return `${message}\n${stack.filter(frame => !noise.test(frame.getFunctionName())).map(renderer).filter(frame => frame).map(function(frame) {
				return `    at ${frame}\n`;
			}).join('')}`;
		}
	}
};
