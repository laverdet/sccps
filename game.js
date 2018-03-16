'use strict';
const ArrayLib = require('array');
const CreepLib = require('creep');
const RoomLib = require('room');
const StructureLib = require('structure');

module.exports = {
	write(env, gamePtr, creepsPtr, resourcesPtr, flagsPtr, sourcesPtr, structuresPtr, tombstonesPtr, mineralPtr) {
		let rooms = Object.keys(Game.rooms);
		let flushes = Array(rooms.length);
		let creepsBegin = creepsPtr + 4;
		let sourcesBegin = sourcesPtr + 4;
		let structuresBegin = structuresPtr + 4;
		for (let ii = rooms.length - 1; ii >= 0; --ii) {
			// Get initial array state
			let room = Game.rooms[rooms[ii]];

			// Push room to arrays
			let creepsEnd = CreepLib.writeToArray(env, creepsPtr, room.find(FIND_CREEPS));
			let sourcesEnd = RoomLib.writeSourcesToArray(env, sourcesPtr, room.find(FIND_SOURCES));
			let structuresEnd = StructureLib.writeToArray(env, structuresPtr, room.find(FIND_STRUCTURES));
			let minerals = room.find(FIND_MINERALS);
			if (minerals.length === 1) {
				RoomLib.writeMineral(env, mineralPtr, minerals[0]);
			} else if (minerals.length === 0) {
				RoomLib.writeMineral(env, mineralPtr, undefined);
			} else {
				console.log('Found more than 1 mineral in room '+ room.roomName);
				RoomLib.writeMineral(env, mineralPtr, minerals[0]);
			}

			// Memoize arguments to flush all at once. I think this might improve v8 performance but
			// didn't actually test it.
			let roomLocation = env.screeps.position.parseRoomName(room.name);
			flushes[ii] = [
				gamePtr,
				roomLocation.rx, roomLocation.ry,
				room.energyAvailable, room.energyCapacityAvailable,
				creepsBegin, creepsEnd,
				0, 0, // dropped
				sourcesBegin, sourcesEnd,
				structuresBegin, structuresEnd,
				0, 0, // tombstone
			];
			creepsBegin = creepsEnd;
			sourcesBegin = sourcesEnd;
			structuresBegin = structuresEnd;
		}

		// Flush room structures
		for (let ii = flushes.length - 1; ii >= 0; --ii) {
			env.__ZN12game_state_t10flush_roomEPS_jjjjPvS1_S1_S1_S1_S1_S1_S1_S1_S1_.apply(env, flushes[ii]);
		}
	},
};
