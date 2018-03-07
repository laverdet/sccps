'use strict';
module.exports = {
	map(env, ptr, length, sizeof, fn) {
		let result = new Array(length);
		let end = ptr + length * sizeof;
		let ii = 0;
		while (ptr < end) {
			result[ii++] = fn(env, ptr);
			ptr += sizeof;
		}
		return result;
	},
};
