'use strict';
const ArrayLib = require('array');
const ObjectLib = require('object');

const structureEnum = Object.freeze([
	STRUCTURE_CONTAINER,
	STRUCTURE_CONTROLLER,
	STRUCTURE_EXTENSION,
	STRUCTURE_EXTRACTOR,
	STRUCTURE_KEEPER_LAIR,
	STRUCTURE_LAB,
	STRUCTURE_LINK,
	STRUCTURE_NUKER,
	STRUCTURE_OBSERVER,
	STRUCTURE_PORTAL,
	STRUCTURE_POWER_BANK,
	STRUCTURE_POWER_SPAWN,
	STRUCTURE_RAMPART,
	STRUCTURE_ROAD,
	STRUCTURE_SPAWN,
	STRUCTURE_STORAGE,
	STRUCTURE_TERMINAL,
	STRUCTURE_TOWER,
	STRUCTURE_WALL,
].reduce(function(map, val, ii) {
	map.set(val, ii);
	return map;
}, new Map));

let sizeof;
let structureLayout, destroyableLayout, ownedLayout;
let spawnLayout;

const that = module.exports = {
	init(layouts) {
		sizeof = layouts.sizeof;
		structureLayout = Object.freeze(layouts.structure);
		destroyableLayout = Object.freeze(layouts.destroyable);
		ownedLayout = Object.freeze(layouts.owned);
		spawnLayout = Object.freeze(layouts.spawn);
	},

	write(env, ptr, structure) {
		ObjectLib.writeGameObject(env, ptr, structure);
		env.HEAPU32[(ptr + structureLayout.structureType) >> 2] = structureEnum.get(structure.structureType);
		switch (structure.structureType) {
			case STRUCTURE_CONTROLLER:
			case STRUCTURE_KEEPER_LAIR:
			case STRUCTURE_PORTAL:
			case STRUCTURE_POWER_BANK:
				// indestrubable
			default:
				// Structure with hit points
				env.HEAPU32[(ptr + destroyableLayout.hits) >> 2] = structure.hits;
				env.HEAPU32[(ptr + destroyableLayout.hitsMax) >> 2] = structure.hitsMax;
				if (structure.structureType === STRUCTURE_ROAD) {
				} else if (structure.structureType === STRUCTURE_WALL) {
				} else {
					// Owned structure
					env.HEAPU8[(ptr + ownedLayout.my) >> 0] = structure.my;
					switch (structure.structureType) {
						case STRUCTURE_SPAWN:
							env.HEAPU32[(ptr + spawnLayout.energy) >> 2] = structure.energy;
							env.HEAPU32[(ptr + spawnLayout.energyCapacity) >> 2] = structure.energyCapacity;
							break;
					}
				}
				break;
		}
	},

	writeToArray(env, arrayPtr, array) {
		for (let ii = array.length - 1; ii >= 0; --ii) {
			that.write(env, ArrayLib.push(env, arrayPtr, sizeof), array[ii]);
		}
		return arrayPtr + 4 + env.HEAPU32[arrayPtr >> 2] * sizeof;
	},
};
