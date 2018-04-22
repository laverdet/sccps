'use strict';
const that = module.exports = {
	// Write entire contents of `array` to `ptr`, using `fn` as a writer
	write(env, ptr, sizeof, capacity, array, fn) {
		if (capacity < array.length) {
			throw new Error('Array overflow');
		}
		let offset = ptr + 4;
		for (let ii = array.length - 1; ii >= 0; --ii) {
			fn(env, offset + ii * sizeof, array[ii]);
		}
		env.HEAPU32[ptr >> 2] = array.length;
	},

	// Append contents of `array` to `ptr`, using `fn` as a writer. Returns pointer to end of array.
	writeAppend(env, ptr, sizeof, capacity, array, fn) {
		let length = env.HEAPU32[ptr >> 2];
		if (capacity < length + array.length) {
			throw new Error('Array overflow');
		}
		let offset = ptr + 4 + length * sizeof;
		for (let ii = array.length - 1; ii >= 0; --ii) {
			fn(env, offset + ii * sizeof, array[ii]);
		}
		env.HEAPU32[ptr >> 2] = length + array.length;
		return offset + array.length * sizeof;
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

	push(env, ptr, sizeof, capacity) {
		let length = env.HEAPU32[ptr >> 2] + 1;
		if (length > capacity) {
			throw new Error('Array overflow');
		}
		env.HEAPU32[ptr >> 2] = length;
		return ptr + 4 + length * sizeof;
	},
};
