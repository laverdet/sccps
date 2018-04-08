'use strict';
const ArrayLib = require('array');
const PositionLib = require('position');
const ResourceLib = require('resource');
const StringLib = require('string');
const kWorldSize = 255;

let droppedResourceLayout;

const that = module.exports = {
	init(layouts) {
		droppedResourceLayout = layouts.droppedResource;
	},

	writeRoomObject(env, ptr, obj) {
		PositionLib.write(env, ptr, obj.pos);
	},

	writeDroppedResource(env, ptr, obj) {
		that.writeGameObject(env, ptr, obj);
		env.HEAPU32[(ptr + droppedResourceLayout.amount) >> 2] = obj.amount;
		env.HEAPU32[(ptr + droppedResourceLayout.resourceType) >> 2] = ResourceLib.jsToC(obj.resourceType);
	},

	writeDroppedResourcesToArray(env, arrayPtr, array) {
		for (let ii = array.length - 1; ii >= 0; --ii) {
			that.writeDroppedResource(env, ArrayLib.push(env, arrayPtr, droppedResourceLayout.sizeof), array[ii]);
		}
		return arrayPtr + 4 + env.HEAPU32[arrayPtr >> 2] * droppedResourceLayout.sizeof;
	},

	writeGameObject(env, ptr, obj) {
		that.writeRoomObject(env, ptr, obj);
		if (obj.id.length > 24) {
			throw new Error('`id` overflow');
		}
		StringLib.writeOneByteString(env, ptr + 4, obj.id);
	},
}
