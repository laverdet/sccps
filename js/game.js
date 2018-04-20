'use strict';
const ArrayLib = require('array');
const CreepLib = require('creep');
const ObjectLib = require('object');
const RoomLib = require('room');
const StructureLib = require('structure');

module.exports = {
	write(env, gamePtr, timePtr, creepsPtr, droppedResourcesPtr, flagsPtr, sourcesPtr, structuresPtr, tombstonesPtr, mineralPtr) {
		let rooms = Object.keys(Game.rooms);
		let flushes = Array(rooms.length);
		let creepsBegin = creepsPtr + 4;
		let droppedResourcesBegin = droppedResourcesPtr + 4;
		let sourcesBegin = sourcesPtr + 4;
		let structuresBegin = structuresPtr + 4;
		let creepsCount = 0, droppedResourcesCount = 0, sourcesCount = 0, structuresCount = 0;
		for (let ii = rooms.length - 1; ii >= 0; --ii) {
			// Get initial array state
			let room = Game.rooms[rooms[ii]];

			// Push room to arrays
			let creeps = room.find(FIND_CREEPS);
			let creepsEnd = CreepLib.writeToArray(env, creepsPtr, creeps);
			creepsCount += creeps.length;
			let droppedResources = room.find(FIND_DROPPED_RESOURCES);
			let droppedResourcesEnd = ObjectLib.writeDroppedResourcesToArray(env, droppedResourcesPtr, droppedResources);
			droppedResourcesCount += droppedResources.length;
			let sources = room.find(FIND_SOURCES);
			let sourcesEnd = RoomLib.writeSourcesToArray(env, sourcesPtr, sources);
			sourcesCount += sources.length;
			let structures = room.find(FIND_STRUCTURES);
			let structuresEnd = StructureLib.writeToArray(env, structuresPtr, structures);
			structuresCount += structures.length;
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
				droppedResourcesBegin, droppedResourcesEnd,
				sourcesBegin, sourcesEnd,
				structuresBegin, structuresEnd,
				0, 0, // tombstone
			];
			creepsBegin = creepsEnd;
			droppedResourcesBegin = droppedResourcesEnd;
			sourcesBegin = sourcesEnd;
			structuresBegin = structuresEnd;
		}

		// Flush room structures
		env.__ZN7screeps12game_state_t13reserve_roomsEPS0_j(gamePtr, flushes.length);
		for (let ii = flushes.length - 1; ii >= 0; --ii) {
			env.__ZN7screeps12game_state_t10flush_roomEPS0_jjjjPvS2_S2_S2_S2_S2_S2_S2_S2_S2_.apply(env, flushes[ii]);
		}

		// Flush game
		env.HEAPU32[timePtr >> 2] = Game.time;
		env.__ZN7screeps12game_state_t10flush_gameEPS0_jjjjj(gamePtr, creepsCount, droppedResourcesCount, sourcesCount, structuresCount, 0);
	},
};
