'use strict';
const that = module.exports = {
	writeOneByteStringData(env, ptr, str) {
		for (let ii = 0; ii < str.length; ++ii) {
			env.HEAPU8[(ptr++) >> 0] = str.charCodeAt(ii);
		}
	},

	writeOneByteString(env, ptr, str) {
		env.HEAPU32[ptr >> 2] = str.length;
		that.writeOneByteStringData(env, ptr + 4, str);
	},

	readOneByteStringData(env, ptr, length) {
		return String.fromCharCode.apply(String, env.HEAPU8.subarray(ptr, ptr + length));
	},

	readOneByteString(env, ptr) {
		return that.readOneByteStringData(env, ptr + 4, env.HEAPU32[ptr >> 2]);
	},
};
