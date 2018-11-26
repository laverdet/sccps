'use strict';
// Copyright (c) 2017 ukyo
// https://github.com/ukyo/zlib-wasm-without-emscripten-sample

// marcel: Removed `deflate` function from runtime and wasm. Added `destroy` option. Further
// modified to work w/ Screeps.
module.exports = function() {
	const COMPRESSION_LEVEL = 6;
	const NO_ZLIB_HEADER = -1;
	const CHUNK_SIZE = 32 * 1024;
	let map = {};
	let memory = new WebAssembly.Memory({
		initial: 1,
		maximum: 1024, // 64MB
	});
	let env = {
		memory,
		writeToJs(ptr, size) {
			const o = map[ptr];
			o.onData(new Uint8Array(memory.buffer, dstPtr, size));
		},
		_abort: errno => { console.error(`Error: ${errno}`) },
		_grow: () => { },
	};

	let module = new WebAssembly.Module(require('inflate.wasm'));
	let exports = (new WebAssembly.Instance(module, { env })).exports;

	const srcPtr = exports._malloc(CHUNK_SIZE);
	const dstPtr = exports._malloc(CHUNK_SIZE);

	class RawInf {
		constructor() {
			this.zstreamPtr = exports._createInflateContext(NO_ZLIB_HEADER);
			map[this.zstreamPtr] = this;
			this.offset = 0;
			this.buff = new Uint8Array(CHUNK_SIZE);
		}

		inflate(chunk) {
			const src = new Uint8Array(memory.buffer, srcPtr, chunk.length);
			src.set(chunk);
			exports._inflate(this.zstreamPtr, srcPtr, dstPtr, chunk.length, CHUNK_SIZE);
		}

		onData(chunk) {
			if (this.buff.length < this.offset + chunk.length) {
				const buff = this.buff;
				this.buff = new Uint8Array(this.buff.length * 2);
				this.buff.set(buff);
			}
			this.buff.set(chunk, this.offset);
			this.offset += chunk.length;
		}

		destroy() {
			exports._freeInflateContext(this.zstreamPtr);
			delete map[this.zstreamPtr];
			this.buff = null;
		}

		getBuffer() {
			return this.buff.subarray(0, this.offset);
		}
	}

	return {
		destroy() {
			env = undefined;
			exports = undefined;
			map = undefined;
			memory = undefined;
			module = undefined;
		},

		inflate(rawDeflateBuffer) {
			const rawInf = new RawInf();
			for (let offset = 0; offset < rawDeflateBuffer.length; offset += CHUNK_SIZE) {
				const end = Math.min(offset + CHUNK_SIZE, rawDeflateBuffer.length);
				const chunk = rawDeflateBuffer.subarray(offset, end);
				rawInf.inflate(chunk);
			}
			const ret = rawInf.getBuffer();
			rawInf.destroy();
			return ret;
		},
	};
}();
