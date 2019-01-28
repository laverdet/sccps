'use strict';
const that = module.exports = {
	writeOneByteStringData(env, ptr, str) {
		for (let ii = 0, length = str.length; ii < length; ++ii) {
			env.HEAPU8[ptr++] = str.charCodeAt(ii);
		}
	},

	writeTwoByteStringData(env, ptr, str) {
		ptr /= 2;
		for (let ii = 0, length = str.length; ii < length; ++ii) {
			env.HEAPU16[ptr++] = str.charCodeAt(ii);
		}
	},

	writeOneByteString(env, ptr, str) {
		env.writeUint32(ptr, str.length);
		that.writeOneByteStringData(env, ptr + env.ptrSize, str);
	},

	readOneByteStringData(env, ptr, length) {
		// Adapted from `Pointer_stringify` in emscripten
		const MAX_CHUNK = 1024; // split up into chunks, because .apply on a huge string can overflow the stack
		let ret = '';
		while (length > 0) {
			let curr = String.fromCharCode.apply(String, env.HEAPU8.subarray(ptr, ptr + Math.min(length, MAX_CHUNK)));
			ret = ret + curr;
			ptr += MAX_CHUNK;
			length -= MAX_CHUNK;
		}
		return ret;
	},

	readTwoByteStringData(env, ptr, length) {
		// Adapted from `Pointer_stringify` in emscripten
		const MAX_CHUNK = 1024; // split up into chunks, because .apply on a huge string can overflow the stack
		let ret = '';
		ptr /= 2; // Do not use >>> because it can overflow on 64bit addresses when compiling as a module
		while (length > 0) {
			let curr = String.fromCharCode.apply(String, env.HEAPU16.subarray(ptr, ptr + Math.min(length, MAX_CHUNK)));
			ret = ret + curr;
			ptr += MAX_CHUNK;
			length -= MAX_CHUNK;
		}
		return ret;
	},

	readUtf8StringData(env, ptr, length) {
		// Check for UTF
		let hasUtf = 0;
		for (let ii = ptr + length - 1; ii >= ptr; --ii) {
			hasUtf |= env.HEAPU8[ii];
		}
		if (hasUtf < 128) {
			return that.readOneByteStringData(env, ptr, length);
		}
		// Adapted from `UTF8ArrayToString` in emscripten
		let u0, u1, u2, u3, u4, u5;
		let end = ptr + length;
		let str = '';
		while (ptr < end) {
			// For UTF8 byte structure, see http://en.wikipedia.org/wiki/UTF-8#Description and https://www.ietf.org/rfc/rfc2279.txt and https://tools.ietf.org/html/rfc3629
			u0 = env.HEAPU8[ptr++];
			if (!(u0 & 0x80)) {
				str += String.fromCharCode(u0);
				continue;
			}
			u1 = env.HEAPU8[ptr++] & 63;
			if ((u0 & 0xe0) == 0xc0) {
				str += String.fromCharCode(((u0 & 31) << 6) | u1);
				continue;
			}
			u2 = env.HEAPU8[ptr++] & 63;
			if ((u0 & 0xf0) == 0xe0) {
				u0 = ((u0 & 15) << 12) | (u1 << 6) | u2;
			} else {
				u3 = env.HEAPU8[ptr++] & 63;
				if ((u0 & 0xf8) == 0xf0) {
					u0 = ((u0 & 7) << 18) | (u1 << 12) | (u2 << 6) | u3;
				} else {
					u4 = env.HEAPU8[ptr++] & 63;
					if ((u0 & 0xfc) == 0xf8) {
						u0 = ((u0 & 3) << 24) | (u1 << 18) | (u2 << 12) | (u3 << 6) | u4;
					} else {
						u5 = env.HEAPU8[ptr++] & 63;
						u0 = ((u0 & 1) << 30) | (u1 << 24) | (u2 << 18) | (u3 << 12) | (u4 << 6) | u5;
					}
				}
			}
			if (u0 < 0x10000) {
				str += String.fromCharCode(u0);
			} else {
				let ch = u0 - 0x10000;
				str += String.fromCharCode(0xd800 | (ch >> 10), 0xdc00 | (ch & 0x3ff));
			}
		}
		return str;
	},

	readOneByteString(env, ptr) {
		return that.readOneByteStringData(env, ptr + env.ptrSize, env.readUint32(ptr));
	},
};
