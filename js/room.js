'use strict';
const ArrayLib = require('array');
const ObjectLib = require('object');
const ResourceLib = require('resource');
const util = require('util');

const [ colorEnum ] = util.enumToMap([
	COLOR_BLUE,
	COLOR_BROWN,
	COLOR_CYAN,
	COLOR_GREEN,
	COLOR_GREY,
	COLOR_ORANGE,
	COLOR_PURPLE,
	COLOR_RED,
	COLOR_WHITE,
	COLOR_YELLOW,
]);

let flagLayout;
let mineralLayout;
let sourceLayout;
let resourceLayout;
let tombstoneLayout;

const that = module.exports = {
	init(layouts) {
		flagLayout = layouts.flag;
		mineralLayout = layouts.mineral;
		sourceLayout = layouts.source;
		resourceLayout = layouts.resource;
		tombstoneLayout = layouts.tombstone;
	},

	writeMineral(env, ptr, mineral) {
		if (mineral === undefined) {
			env.HEAPU32[(ptr + mineralLayout.type) >> 2] = 0;
		} else {
			ObjectLib.writeGameObject(env, ptr, mineral);
			env.HEAPU32[(ptr + mineralLayout.type) >> 2] = ResourceLib.resourceEnum.get(mineral.mineralType);
			env.HEAPU32[(ptr + mineralLayout.amount) >> 2] = mineral.mineralAmount;
			env.HEAPU32[(ptr + mineralLayout.density) >> 2] = mineral.density;
			env.HEAPU32[(ptr + mineralLayout.ticksToRegeneration) >> 2] = mineral.ticksToRegeneration;
		}
	},

	writeSource(env, ptr, source) {
		ObjectLib.writeGameObject(env, ptr, source);
		env.HEAPU32[(ptr + sourceLayout.energy) >> 2] = source.energy;
		env.HEAPU32[(ptr + sourceLayout.energyCapacity) >> 2] = source.energyCapacity;
		env.HEAPU32[(ptr + sourceLayout.ticksToRegeneration) >> 2] = source.ticksToRegeneration;
	},

	writeSourcesToArray(env, arrayPtr, array) {
		for (let ii = array.length - 1; ii >= 0; --ii) {
			that.writeSource(env, ArrayLib.push(env, arrayPtr, sourceLayout.sizeof), array[ii]);
		}
		return arrayPtr + 4 + env.HEAPU32[arrayPtr >> 2] * sourceLayout.sizeof;
	},
};
