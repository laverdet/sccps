'use strict';
let { flush, print } = function() {
	const kMaxLinesPerTick = 90;
	const kShowOmittedLines = 10;
	let lineCount, skippedCount;
	let lastLine, lastStream;
	let lineRepeat;
	let skipTick;
	let omittedLines;

	function reset() {
		lineCount = 0;
		lastLine = undefined;
		lineRepeat = 0;
		skippedCount = 0;
		skipTick = Game.time
	}

	function printLine(stream, line) {
		if (stream === 0) {
			console.log(`<span style="white-space:normal">${line}</span>`);
		} else if (stream === 2) {
			console.log(`<span style="white-space:normal;color:#e79da7">${line}</span>`);
		} else {
			throw new Error(`Unknown output stream ${stream}`);
		}
	}

	function notifyRepeat(count, lastTick) {
		console.log(`<span style="color:#ffc66d">[Line repeated <b>${count}</b> time${count > 1 ? 's' : ''}${lastTick ? ' on previous tick' : ''}]</span>`);
	}

	reset();
	return {
		print(stream, line) {
			line = `${line}`;
			if (lineCount < kMaxLinesPerTick) {
				if (line === lastLine && lastStream === stream) {
					++lineRepeat;
					return;
				} else if (lineRepeat > 0) {
					if (lineRepeat === 1) {
						printLine(stream, lastLine);
					} else {
						notifyRepeat(lineRepeat, false);
					}
					lastLine = undefined;
					lineRepeat = 0;
				}
			}
			if (++lineCount > kMaxLinesPerTick) {
				if (omittedLines === undefined) {
					omittedLines = [{ stream, line }];
				} else {
					omittedLines.push({ stream, line });
					if (omittedLines.length > kShowOmittedLines * 2) {
						omittedLines.splice(0, kShowOmittedLines);
					}
				}
				++skippedCount;
			} else {
				printLine(stream, line);
				lastLine = line;
				lastStream = stream;
			}
		},

		flush() {
			if (lineRepeat === 1) {
				printLine(lastStream, lastLine);
			} else if (lineRepeat > 0) {
				notifyRepeat(lineRepeat, skipTick !== Game.time);
			} else if (skippedCount > 0) {
				let showOmittedCount = Math.max(0, omittedLines.length - kShowOmittedLines);
				skippedCount -= showOmittedCount;
				if (skippedCount > 0) {
					console.log(`<span style="color:#ffc66d">[Omitted <b>${skippedCount}</b> line${skippedCount > 1 ? 's' : ''}${Game.time !== skipTick ? ' on previous tick' : ''}]</span>`);
				}
				for (let ii = Math.max(0, omittedLines.length - kShowOmittedLines); ii < omittedLines.length; ++ii) {
					printLine(omittedLines[ii].stream, omittedLines[ii].line);
				}
				if (Game.time !== skipTick) {
					console.log(`<span style="color:#ffc66d">[Output from current tick follows]</span>`);
				}
			}
			reset();
		}
	};
}();

function bufferToString(buffer) {
	const kChunkSize = 4096;
	let offset = 0;
	let length = buffer.length;
	let ret = '';
	while (offset < buffer.length) {
		ret += String.fromCharCode.apply(String, buffer.subarray(offset, offset + Math.min(buffer.length - offset, kChunkSize)));
		offset += kChunkSize;
	}
	return ret;
}

function maybeBufferToString(value) {
	return typeof value === 'string' ? value : bufferToString(value);
}

function mangleFileName(name) {
	return name.replace(/[\/.\-]/g, '__');
}

function evalWithName(evil, name, src) {
	return evil(`(function ${mangleFileName(name)}(evil, src){return evil.call(global, src);})`)(evil, src);
}

global.trace = function trace(message) {
	let stack = { message };
	Error.captureStackTrace(stack, trace);
	console.log(stack.stack);
}

let asmjsArchive, buffer;
let didExitCleanly = true;
print(2, 'Global reset');
function* initialize() {

	// Try to load inflate
	let inflate;
	try {
		inflate = require('inflate');
		yield 'load:inflate';
	} catch (err) {}

	// Try to load source map parser
	let SourceMapConsumer;
	try {
		SourceMapConsumer = require('source-map').SourceMapConsumer;
		yield 'load:sourceMap';
	} catch (err) {}

	// Holders for runtime data
	let runtime, wasm;
	let sourceMaps = Object.create(null);
	let dylibs = Object.create(null);

	// Extract custom archive format
	let info = require('the_general');
	let isAsmjs = true;
	if (info instanceof Uint8Array) {
		let view = new DataView(info.buffer, info.byteOffset, info.byteLength);
		let ptr = 0;
		while (ptr != info.length) {
			// Extract file and name
			let len = view.getUint32(ptr);
			ptr += 4;
			let name = String.fromCharCode.apply(String, info.subarray(ptr, ptr + len));
			ptr += len;
			len = view.getUint32(ptr);
			ptr += 4;
			let data = info.subarray(ptr, ptr + len);
			ptr += len;
			// Inflate if data is compressed
			if (name.substr(-8) === '.deflate') {
				name = name.substr(0, name.length - 8);
				data = inflate.inflate(data);
			}
			// Handle files
			if (name === 'asmjs.js' || name === 'wasm.js') {
				runtime = function(module) {
					global.Module = module;
					evalWithName(eval, name, maybeBufferToString(data));
					return module;
				};
			} else if (name === 'wasm.wasm') {
				isAsmjs = false;
				wasm = data;
			} else if (name.substr(-9) === '.dylib.js') {
				dylibs[name.substr(0, name.length - 3)] = maybeBufferToString(data);
			} else if (name.substr(-11) === '.dylib.wasm') {
				dylibs[name.substr(0, name.length - 5)] = data;
			} else if (name.substr(-4) === '.map') {
				sourceMaps[mangleFileName(name.substr(0, name.length - 4))] = new SourceMapConsumer(JSON.parse(maybeBufferToString(data)));
			} else {
				throw new Error(`Unhandled file: ${name}`);
			}
		}
	}
	yield 'extract';

	// Load our modules
	const screeps = {
		array: require('array'),
		object: require('object'),
		position: require('position'),
		string: require('string'),
		util: require('util'),
		vector: require('vector'),
	};
	yield 'lib';

	// Missing function =o
	global.invoke_X = function(...args) {
		return Module['wasmTable'].get(args[0]).apply(null, args.slice(1));
	}

	// Start the runtime
	if (isAsmjs) {
		buffer = buffer || new ArrayBuffer(201326592);
	}
	const mod = runtime(global.Module = {
		noInitialRun: true,
		noExitRuntime: true,
		print: line => print(0, line),
		printErr: line => print(2, line),
		onAbort: function(what) {
			throw new Error(what);
		},
		read: function(lib) {
			return dylibs[lib];
		},
		readBinary: function(lib) {
			return dylibs[lib];
		},
		screeps,
		buffer,
		wasmBinary: wasm,
	});
	mod.__ZN7screeps3cpu18adjust_memory_sizeEi(STATICTOP);
	yield 'runtime';

	// Setup exception handler
	new function() {
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
	};

	// Link dylibs
	{
		let currentLib;
		let evil = eval;
		global.eval = function(src) {
			// Ensures library name makes it to stack traces for source maps to work
			return evalWithName(evil, currentLib, src);
		};
		for (let lib in dylibs) {
			currentLib = lib;
			// Dynamic libraries have a dynamic stack which counts against total memory. But it only
			// affects dynamic builds so we adjust downwards that way heap stats are predictable during
			// development.
			mod.__ZN7screeps3cpu18adjust_memory_sizeEi(-TOTAL_STACK);
			mod.loadDynamicLibrary(lib);
			yield lib;
		}
		global.eval = evil;
	}

	// Initialize internal structures
	try {
		mod.__ZN7screeps12game_state_t11init_layoutEv();
	} catch (err) {
		print(2, `Uncaught error in init():\n${err.stack || err.message || err}`);
		throw err;
	}

	// Collect garbage
	if (inflate) {
		inflate.destroy();
		inflate = undefined;
	}
	asmjsArchive = dylibs = runtime = wasm = info = SourceMapConsumer = undefined;
	gc();
	yield 'gc';
	yield `!Heap: ${Game.cpu.getHeapStatistics().used_heap_size / 1024 / 1024}mb`;

	// Main loop starts here
	return function() {
		flush();
		if (!didExitCleanly) {
			print(2, 'Engine is in undefined state, refusing to run loop');
			return;
		}
		if (Game.cpu.limit + Game.cpu.bucket === Game.cpu.tickLimit) {
			print(2, 'Skipping loop due to empty bucket.');
			return;
		}
		try {
			didExitCleanly = false;
			mod.__Z4loopv();
			didExitCleanly = true;
		} catch (err) {
			if (typeof err === 'number') {
				/**
				 * Modifications needed to emscripten!
				 *
				 * File: emscripten/<version>/src/library.js
				 *
				 * In `__cxa_throw` modify the line that reads `EXCEPTIONS.infos[ptr] = {...}` to this:
				 * Error.captureStackTrace(Module.lastException = EXCEPTIONS.infos[ptr] = {...});
				 */
				throw mod.lastException;
			} else {
				throw err;
			}
		} finally {
			flush();
		}
	};
}

// Run the initialiization over several ticks if necessary, to avoid CPU errors
let initialization = initialize();
let messages;
module.exports.loop = function() {
	flush();
	if (Game.cpu.limit + Game.cpu.bucket === Game.cpu.tickLimit) {
		print(2, 'Deferring initialization due to empty bucket.');
		return;
	}
	if (messages && messages.length !== 0) {
		print(0, messages.join('... '));
	}
	let totalTime = 0;
	let started = Date.now();
	messages = [];
	let extraMessages = [];
	try {
		do {
			let element = initialization.next();
			if (element.done) {
				module.exports.loop = element.value;
				return;
			} else {
				if (element.value.charAt(0) === '!') {
					extraMessages.push(element.value.substr(1));
				} else {
					let now = Date.now();
					let thisTime = now - started;
					totalTime += thisTime;
					messages.push(`${element.value}: ${thisTime}ms`);
					started = now;
					if (totalTime > 250) {
						return;
					}
				}
			}
		} while (true);
	} catch (err) {
		if (/Generator is already running/.test(err.message)) {
			print(2, 'Initialization timed out. Restarting.');
			initialization = initialize();
			module.exports.loop();
		} else {
			print(2, `Caught error during initialization:\n${err.stack || err.message || err}`);
			initialization = initialize();
		}
	} finally {
		if (messages && messages.length !== 0) {
			print(0, messages.join('... '));
		}
		messages = undefined;
		extraMessages.map(message => print(0, extraMessages));
		flush();
	}
};
