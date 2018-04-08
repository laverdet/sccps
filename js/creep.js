'use strict';
const ArrayLib = require('array');
const ObjectLib = require('object');
const ResourceLib = require('resource');
const StringLib = require('string');
const util = require('util');

const [ bodyPartEnumReverse, bodyPartEnum ] = util.enumToMap([
	ATTACK,
	CARRY,
	CLAIM,
	HEAL,
	MOVE,
	RANGED_ATTACK,
	TOUGH,
	WORK,
]);

let sizeof;
let creepLayout;

const that = module.exports = {
	init(layout) {
		sizeof = layout.sizeof;
		creepLayout = Object.freeze(layout);
	},

	readBodyPartList(env, ptr) {
		return ArrayLib.map(env, ptr, 4, function(env, ptr) {
			return bodyPartEnum.get(env.HEAPU32[ptr >> 2]);
		});
	},

	write(env, ptr, creep) {
		ObjectLib.writeGameObject(env, ptr, creep);
		ResourceLib.write(env, ptr + creepLayout.carry, creep.carry);
		env.HEAPU32[(ptr + creepLayout.carryCapacity) >> 2] = creep.carryCapacity;
		env.HEAPU32[(ptr + creepLayout.fatigue) >> 2] = creep.fatigue;
		env.HEAPU32[(ptr + creepLayout.hits) >> 2] = creep.hits;
		env.HEAPU32[(ptr + creepLayout.hitsMax) >> 2] = creep.hitsMax;
		env.HEAP8[(ptr + creepLayout.my) >> 0] = creep.my ? 1 : 0;
		if (creep.my) {
			StringLib.writeOneByteString(env, ptr + creepLayout.name, creep.name);
		} else {
			env.HEAPU32[(ptr + creepLayout.name) >> 2] = 0;
		}
		env.HEAP8[(ptr + creepLayout.spawning) >> 0] = creep.spawning ? 1 : 0;
		env.HEAPU32[(ptr + creepLayout.ticksToLive) >> 2] = creep.ticksToLive;
	},

	writeToArray(env, arrayPtr, array) {
		for (let ii = array.length - 1; ii >= 0; --ii) {
			that.write(env, ArrayLib.push(env, arrayPtr, sizeof), array[ii]);
		}
		return arrayPtr + 4 + env.HEAPU32[arrayPtr >> 2] * sizeof;
	},
};
