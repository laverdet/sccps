'use strict';
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

module.exports = {
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
			env.HEAPU32[(ptr + mineralLayout.type) >> 2] = mineral.mineralType;
			env.HEAPU32[(ptr + mineralLayout.amount) >> 2] = mineral.mineralAmount;
			env.HEAPU32[(ptr + mineralLayout.density) >> 2] = mineral.density;
			env.HEAPU32[(ptr + mineralLayout.ticksToRegeneration) >> 2] = mineral.ticksToRegeneration;
		}
	},
};
