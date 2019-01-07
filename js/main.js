'use strict';
let { flush, print } = require('console');

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

// Handles proxy objects which act like emscripten's HEAP* objects but over a 64-bit range. Only works up to SMI 53-bit though..
function makeHeapProxies(mod) {
	let map = new Map;
	function getArrayBuffer(ptr) {
		let val = map.get(ptr);
		if (val === undefined) {
			map.set(ptr, val = mod.makeArrayBuffer(ptr));
		}
		return val;
	}

	function makeProxy(ctor, size) {
		// Max typed array length is 0x7fffffff
		const segmentSize = 0x40000000;
		let map = new Map;
		function getView(index) {
			index = Math.floor(index / segmentSize);
			let val = map.get(index);
			if (val === undefined) {
				let ab = getArrayBuffer(index * segmentSize * (1 << (size - 1)));
				map.set(index, val = new ctor(ab, 0, segmentSize));
			}
			return val;
		}
		return new Proxy({
			subarray(offset, end) {
				let ab = getView(offset);
				if (ab !== getView(end)) {
					throw new Error('Subarray slices two segments');
				}
				return ab.subarray(offset % segmentSize, end % segmentSize);
			},
		}, {
			get(that, prop) {
				let index = Number(prop);
				if (index === index) {
					return getView(index)[index % segmentSize];
				} else {
					return that[prop];
				}
			},
			set(that, prop, val) {
				let index = Number(prop);
				if (index === index) {
					getView(index)[index % segmentSize] = val;
					return true;
				} else {
					return false;
				}
			},
		});
	}

	mod.HEAP8 = makeProxy(Int8Array, 1);
	mod.HEAPU8 = makeProxy(Uint8Array, 1);
	mod.HEAP16 = makeProxy(Int16Array, 2);
	mod.HEAPU16 = makeProxy(Uint16Array, 2);
	mod.HEAP32 = makeProxy(Int32Array, 3);
	mod.HEAPU32 = makeProxy(Uint32Array, 3);
	mod.HEAPF32 = makeProxy(Float32Array, 3);
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
	} else {
		runtime = info;
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

	let mod;
	if (runtime instanceof Function) { // Is an emscripten module, otherwise it's native
		// Missing function =o
		global.invoke_X = function(...args) {
			return Module['wasmTable'].get(args[0]).apply(null, args.slice(1));
		}

		// Start the runtime
		if (isAsmjs) {
			buffer = buffer || new ArrayBuffer(201326592);
		}
		mod = runtime(global.Module = {
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

			readInt8: addr => mod.HEAP8[addr],
			writeInt8: (addr, val) => void(mod.HEAP8[addr] = val),
			readUint8: addr => mod.HEAPU8[addr],
			writeUint8: (addr, val) => void(mod.HEAPU8[addr] = val),

			readInt16: addr => mod.HEAP16[addr >> 1],
			writeInt16: (addr, val) => void(mod.HEAP16[addr >> 1] = val),
			readUint16: addr => mod.HEAPU16[addr >> 1],
			writeUint16: (addr, val) => void(mod.HEAPU16[addr >> 1] = val),

			readInt32: addr => mod.HEAP32[addr >> 2],
			writeInt32: (addr, val) => void(mod.HEAP32[addr >> 2] = val),
			readUint32: addr => mod.HEAPU32[addr >> 2],
			writeUint32: (addr, val) => void(mod.HEAPU32[addr >> 2] = val),

			readFloat32: addr => mod.HEAPF32[addr >> 2],
			writeFloat32: (addr, val) => void(mod.HEAPF32[addr >> 2] = val),

			ptrSize: 4,
			readPtr: addr => mod.HEAPU32[addr >> 2],
			writePtr: (addr, val) => void(mod.HEAPU32[addr >> 2] = val),
		});
		mod.__ZN7screeps3cpu18adjust_memory_sizeEi(STATICTOP);
		require('error').setupExceptionHandler(mod, sourceMaps);
		yield 'runtime';

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
	} else {
		global.Module = mod = runtime;
		makeHeapProxies(mod);
		Object.assign(mod, {
			screeps,

			readInt8: addr => mod.HEAP8[addr],
			writeInt8: (addr, val) => void(mod.HEAP8[addr] = val),
			readUint8: addr => mod.HEAPU8[addr],
			writeUint8: (addr, val) => void(mod.HEAPU8[addr] = val),

			readInt16: addr => mod.HEAP16[addr / 2],
			writeInt16: (addr, val) => void(mod.HEAP16[addr / 2] = val),
			readUint16: addr => mod.HEAPU16[addr / 2],
			writeUint16: (addr, val) => void(mod.HEAPU16[addr / 2] = val),

			readInt32: addr => mod.HEAP32[addr / 4],
			writeInt32: (addr, val) => void(mod.HEAP32[addr / 4] = val),
			readUint32: addr => mod.HEAPU32[addr / 4],
			writeUint32: (addr, val) => void(mod.HEAPU32[addr / 4] = val),

			readFloat32: addr => mod.HEAPF32[addr / 4],
			writeFloat32: (addr, val) => void(mod.HEAPF32[addr / 4] = val),

			ptrSize: 8,
			readPtr(addr) {
				addr /= 4;
				return mod.HEAPU32[addr + 1] * 0x100000000 + mod.HEAPU32[addr];
			},
			writePtr(addr, val) {
				addr /= 4;
				mod.HEAPU32[addr + 1] = Math.floor(val / 0x100000000);
				mod.HEAPU32[addr] = val % 0x100000000;
			},
		});
		mod.screeps = screeps;
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
	asmjsArchive = dylibs = runtime = wasm = info = SourceMapConsumer = sourceMaps = undefined;
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
