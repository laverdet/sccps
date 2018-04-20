'use strict';
let { flush, print } = function() {
	const kMaxLinesPerTick = 90;
	const kShowOmittedLines = 10;
	let lineCount, skippedCount;
	let lastLine, lastStream;
	let lineRepeat;
	let skipTick;
	let omittedLines;

	function printLine(stream, line) {
		if (stream === 0) {
			console.log(line);
		} else if (stream === 2) {
			console.log(`<span style="color:#e79da7">${line}</span>`);
		} else {
			throw new Error(`Unknown output stream ${stream}`);
		}
	}

	function notifyRepeat(count, lastTick) {
		console.log(`<span style="color:#ffc66d">[Line repeated <b>${count}</b> time${count > 1 ? 's' : ''}${lastTick ? ' on previous tick' : ''}]</span>`);
	}

	return {
		print(stream, line) {
			line = `${line}`;
			if (line === lastLine && lastStream === stream) {
				++lineRepeat;
				return;
			} else if (lineRepeat > 0) {
				if (lineRepeat === 1) {
					printLine(stream, line);
				} else {
					notifyRepeat(lineRepeat, false);
				}
				lineRepeat = 0;
				lastLine = undefined;
				++lineCount;
				return;
			}
			if (++lineCount > kMaxLinesPerTick) {
				if (omittedLines === undefined) {
					omittedLines = [ line ];
				} else {
					omittedLines.push({ stream, line });
					if (omittedLines.length > kShowOmittedLines * 2) {
						omittedLines.splice(0, kShowOmittedLines);
					}
				}
				++skippedCount;
			} else {
				printLine(stream, line);
			}
			lastLine = line;
			lastStream = stream;
		},

		flush() {
			if (lineRepeat > 0) {
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
			lineCount = 0;
			lastLine = undefined;
			lineRepeat = 0;
			skippedCount = 0;
			skipTick = Game.time
		}
	};
}();
flush();

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

function* initialize() {

	// Try to load inflate
	let inflate;
	try {
		inflate = require('inflate');
		yield 'load-inflate';
	} catch (err) {}

	// Load compiled module
	let runtime, wasm, sourceMap;
	try {
		// WebAssembly
		runtime = require('the_general');
		wasm = require('the_general-wasm');
		yield 'wasm';
	} catch (err) {
		// asm.js
		runtime = require('the_general-asmjs');
		yield 'asmjs';
		if (runtime instanceof Uint8Array) {
			let module = { exports: {} };
			(new Function('exports', 'module', bufferToString(inflate.inflate(runtime)))).call(module, module.exports, module);
			runtime = module.exports;
			yield 'inflate-runtime';
		}
		// Look for source map (only for asm.js)
		try {
			let sourceMapDeflated = require('the_general-map');
			yield 'map';
			let sourceMapJson = JSON.parse(bufferToString(inflate.inflate(sourceMapDeflated)));
			yield 'map-inflate';
			let SourceMapConsumer = require('source-map').SourceMapConsumer;
			sourceMap = new SourceMapConsumer(sourceMapJson);
			yield 'map-parse';
		} catch (err) {}
	}

	// Load our modules
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
	yield 'lib';

	// Initialize runtime
	const mod = runtime({
		ENVIRONMENT: 'SHELL',
		wasmBinary: wasm,
		print: line => print(0, line),
		printErr: line => print(2, line),
		noInitialRun: true,
		noExitRuntime: true,
		screeps: screeps,
	});
	yield 'runtime';

	// Initialize internal structures
	try {
		mod.__ZN7screeps12game_state_t4initEv();
	} catch (err) {
		print(2, `Uncaught error in init():\n${err.stack || err.message || err}`);
		throw err;
	}
	let demangleHole = mod._malloc(256 + 4 + 1);

	// Setup exception handler
	new function() {
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
						let ch = mod.HEAP8[ptr++];
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
			if (sourceMap) {
				renderer = function(frame) {
					let fnName = frame.getFunctionName();
					let demangled = demangle(fnName);
					let source;
					let line = frame.getLineNumber() - 5; // Messy function bloat
					let column = frame.getColumnNumber();
					if (fnName !== demangled) {
						let position = sourceMap.originalPositionFor({ line, column });
						if (position.source === null) {
							let origin = frame.getEvalOrigin() || `${frame.getScriptNameOrSourceURL()}:${frame.getLineNumber()}:${frame.getColumnNumber()}`;
							return `${demangled} (${origin})`;
						} else {
							return `${demangled} (${position.source}:${position.line})`;
						}
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
			return `${message}\n${stack.filter(frame => !/^(?:dynCall_|invoke_|_emscripten_asm_const_)[dfiv]+$|^___cxa_throw/.test(frame.getFunctionName())).map(function(frame) {
				return `    at ${renderer(frame)}\n`;
			}).join('')}`;
		}
	};

	// Collect garbage
	if (inflate) {
		inflate.destroy();
		inflate = undefined;
	}
	runtime = undefined;
	gc();
	yield 'gc';
	yield `!Heap: ${Game.cpu.getHeapStatistics().used_heap_size / 1024 / 1024}mb`;

	// Main loop starts here
	return function() {
		flush();
		if (Game.cpu.limit + Game.cpu.bucket === Game.cpu.tickLimit) {
			print(2, 'Skipping loop due to empty bucket.');
			return;
		}
		try {
			mod.__Z4loopv();
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
		print(0, 'Deferring initialization due to empty bucket.');
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
