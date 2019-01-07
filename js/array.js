'use strict';
const that = module.exports = {
	// Write entire contents of `array` to `ptr`, using `fn` as a writer
	write(env, ptr, sizeof, capacity, array, fn) {
		if (capacity < array.length) {
			throw new Error('Array overflow');
		}
		that.writeData(env, ptr + env.ptrSize, sizeof, array, fn);
		env.writeUint32(ptr, array.length);
	},

	// Append contents of `array` to `ptr`, using `fn` as a writer. Returns pointer to end of array.
	writeAppend(env, ptr, sizeof, capacity, array, fn) {
		let length = env.readUint32(ptr);
		if (capacity < length + array.length) {
			throw new Error('Array overflow');
		}
		that.writeData(env, ptr + env.ptrSize + length * sizeof, sizeof, array, fn);
		env.writeUint32(ptr, length + array.length);
		return offset + array.length * sizeof;
	},

	// Plain data writer, doesn't update a `size` value
	writeData(env, ptr, sizeof, array, fn) {
		for (let ii = array.length - 1; ii >= 0; --ii) {
			fn(env, ptr + ii * sizeof, array[ii]);
		}
	},

	map(env, ptr, sizeof, fn) {
		let length = env.readUint32(ptr);
		let array = new Array(length);
		let offset = ptr + env.ptrSize;
		for (let ii = 0; ii < length; ++ii) {
			array[ii] = fn(env, offset);
			offset += sizeof;
		}
		return array;
	},

	push(env, ptr, sizeof, capacity) {
		let length = env.readUint32(ptr) + 1;
		if (length > capacity) {
			throw new Error('Array overflow');
		}
		env.writeUint32(ptr, length);
		return ptr + env.ptrSize + length * sizeof;
	},
};
