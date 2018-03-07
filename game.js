'use strict';
const ArrayLib = require('array');
const CreepLib = require('creep');
const StructureLib = require('structure');

module.exports = {
	read(env, creepsPtr, structuresPtr) {
		let rooms = Object.keys(Game.rooms);
		for (let ii = rooms.length - 1; ii >= 0; --ii) {
			let room = Game.rooms[rooms[ii]];
			CreepLib.writeToArray(env, creepsPtr, room.find(FIND_CREEPS));
			StructureLib.writeToArray(env, structuresPtr, room.find(FIND_STRUCTURES));
		}
	},
};
