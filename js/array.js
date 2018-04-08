'use strict';
module.exports = {
	write(env, ptr, sizeof, capacity, generator, fn) {
		let offset = ptr + 4;
		for (let item of generator) {
			fn(env, offset, item);
			offset += sizeof;
		}
		let length = (offset - ptr - 4) / sizeof;
		if (length > capacity) {
			throw new Error('Array overflow');
		}
		env.HEAPU32[ptr >> 2] = length;
	},

	read(env, ptr, sizeof, fn) {
		let end = ptr + 4 + env.HEAPU32[ptr >> 2] * sizeof;
		for (let offset = ptr + 4; offset < end; offset += sizeof) {
			fn(env, offset);
		}
	},

	map(env, ptr, sizeof, fn) {
		let length = env.HEAPU32[ptr >> 2];
		let array = new Array(length);
		let offset = ptr + 4;
		for (let ii = 0; ii < length; ++ii) {
			array[ii] = fn(env, offset);
			offset += sizeof;
		}
		return array;
	},

	push(env, ptr, sizeof) {
		let length = env.HEAPU32[ptr >> 2];
		env.HEAPU32[ptr >> 2] = length + 1;
		return ptr + 4 + length * sizeof;
	},
};
