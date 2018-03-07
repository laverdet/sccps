'use strict';
const PositionLib = require('position');
const StringLib = require('string');
const kWorldSize = 255;

const exports = module.exports = {
	writeRoomObject(env, ptr, obj) {
		PositionLib.write(env, ptr, obj.pos);
	},

	writeGameObject(env, ptr, obj) {
		exports.writeRoomObject(env, ptr, obj);
		if (obj.id.length > 24) {
			throw new Error('`id` overflow');
		}
		StringLib.writeOneByteString(env, ptr + 4, obj.id);
	},
};
