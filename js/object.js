'use strict';
const ArrayLib = require('array');
const PositionLib = require('position');
const StringLib = require('string');
const util = require('util');
const kWorldSize = 255;

// game_state_t
let gameConstructionSites, gameCreeps, gameDroppedResources, gameFlags, gameSources, gameStructures, gameTombstones;
let gameGcl, gameMineralHole, gameTime;

// resource_store_t
const [ resourceEnum, resourceReverseEnum ] = util.enumToMap([
	null,
	RESOURCE_ENERGY,
	RESOURCE_POWER,

	RESOURCE_HYDROGEN,
	RESOURCE_OXYGEN,
	RESOURCE_UTRIUM,
	RESOURCE_LEMERGIUM,
	RESOURCE_KEANIUM,
	RESOURCE_ZYNTHIUM,
	RESOURCE_CATALYST,
	RESOURCE_GHODIUM,

	RESOURCE_HYDROXIDE,
	RESOURCE_ZYNTHIUM_KEANITE,
	RESOURCE_UTRIUM_LEMERGITE,

	RESOURCE_UTRIUM_HYDRIDE,
	RESOURCE_UTRIUM_OXIDE,
	RESOURCE_KEANIUM_HYDRIDE,
	RESOURCE_KEANIUM_OXIDE,
	RESOURCE_LEMERGIUM_HYDRIDE,
	RESOURCE_LEMERGIUM_OXIDE,
	RESOURCE_ZYNTHIUM_HYDRIDE,
	RESOURCE_ZYNTHIUM_OXIDE,
	RESOURCE_GHODIUM_HYDRIDE,
	RESOURCE_GHODIUM_OXIDE,

	RESOURCE_UTRIUM_ACID,
	RESOURCE_UTRIUM_ALKALIDE,
	RESOURCE_KEANIUM_ACID,
	RESOURCE_KEANIUM_ALKALIDE,
	RESOURCE_LEMERGIUM_ACID,
	RESOURCE_LEMERGIUM_ALKALIDE,
	RESOURCE_ZYNTHIUM_ACID,
	RESOURCE_ZYNTHIUM_ALKALIDE,
	RESOURCE_GHODIUM_ACID,
	RESOURCE_GHODIUM_ALKALIDE,

	RESOURCE_CATALYZED_UTRIUM_ACID,
	RESOURCE_CATALYZED_UTRIUM_ALKALIDE,
	RESOURCE_CATALYZED_KEANIUM_ACID,
	RESOURCE_CATALYZED_KEANIUM_ALKALIDE,
	RESOURCE_CATALYZED_LEMERGIUM_ACID,
	RESOURCE_CATALYZED_LEMERGIUM_ALKALIDE,
	RESOURCE_CATALYZED_ZYNTHIUM_ACID,
	RESOURCE_CATALYZED_ZYNTHIUM_ALKALIDE,
	RESOURCE_CATALYZED_GHODIUM_ACID,
	RESOURCE_CATALYZED_GHODIUM_ALKALIDE,
]);

let extendedResourceStoreSizeof;
let extendedResourceStorePtr;

// construction_site_t
let constructionSiteSizeof;
let constructionSiteMy, constructionSiteProgress, constructionSiteProgressTotal, constructionSiteType;

// creep_t
const [ bodyPartEnum, bodyPartEnumReverse ] = util.enumToMap([
	ATTACK,
	CARRY,
	CLAIM,
	HEAL,
	MOVE,
	RANGED_ATTACK,
	TOUGH,
	WORK,
]);
let creepSizeof;
let creepCarry, creepCarryCapacity, creepFatigue, creepHits, creepHitsMax, creepMy, creepName, creepSpawning, creepTicksToLive;

// dropped_resource_t
let droppedResourceSizeof;
let droppedResourceAmount, droppedResourceType;

// flag_t
const [ colorEnum, colorEnumReverse ] = util.enumToMap([
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

// mineral_t
let mineralSizeof;
let mineralAmount, mineralDensity, mineralType, mineralTicksToRegeneration;

// source_t
let sourceSizeof;
let sourceEnergy, sourceEnergyCapacity, sourceTicksToRegeneration;

// structure_t
const [ structureTypeEnum, structureTypeEnumReverse ] = util.enumToMap([
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
]);
let structureSizeof;
let structureStructureType;
let structureDestroyableHits, structureDestroyableHitsMax;
let structureOwnedMy;
let structureExtensionEnergy, structureExtensionEnergyCapacity;
let structureSpawnEnergy, structureSpawnEnergyCapacity;

const that = module.exports = {
	initGameLayout(layout) {
		gameConstructionSites = layout.constructionSites;
		gameCreeps = layout.creeps;
		gameDroppedResources = layout.droppedResources;
		gameFlags = layout.flags;
		gameSources = layout.sources;
		gameStructures = layout.structures;
		gameTombstones = layout.tombstones;

		gameGcl = layout.gcl;
		gameMineralHole = layout.mineralHole;
		gameTime = layout.time;
	},

	initConstructionSiteLayout(layout) {
		constructionSiteSizeof = layout.sizeof;
		constructionSiteMy = layout.my;
		constructionSiteProgress = layout.progress;
		constructionSiteProgressTotal = layout.progressTotal;
		constructionSiteType = layout.structureType;
	},

	initCreepLayout(layout) {
		creepSizeof = layout.sizeof;
		creepCarry = layout.carry;
		creepCarryCapacity = layout.carryCapacity;
		creepFatigue = layout.fatigue;
		creepHits = layout.hits;
		creepHitsMax = layout.hitsMax;
		creepMy = layout.my;
		creepName = layout.name;
		creepSpawning = layout.spawning;
	},

	initDroppedResourceLayout(layout) {
		droppedResourceSizeof = layout.sizeof;
		droppedResourceAmount = layout.amount;
		droppedResourceType = layout.resourceType;
	},

	initMineralLayout(layout) {
		mineralSizeof = layout.sizeof;
		mineralAmount = layout.amount;
		mineralDensity = layout.density;
		mineralType = layout.mineralType;
		mineralTicksToRegeneration = layout.ticksToRegeneration;
	},

	initResourceStoreLayout(sizeof, ptr) {
		extendedResourceStoreSizeof = sizeof;
		extendedResourceStorePtr = ptr;
	},

	initSourceLayout(layout) {
		sourceSizeof = layout.sizeof;
		sourceEnergy = layout.energy;
		sourceEnergyCapacity = layout.energyCapacity;
		sourceTicksToRegeneration = layout.ticksToRegeneration;
	},

	initStructureLayout(layout) {
		structureSizeof = layout.sizeof;
		structureStructureType = layout.structure.structureType;
		structureDestroyableHits = layout.destroyable.hits;
		structureDestroyableHitsMax = layout.destroyable.hitsMax;
		structureOwnedMy = layout.owned.my;
		structureExtensionEnergy = layout.extension.energy;
		structureExtensionEnergyCapacity = layout.extension.energyCapacity;
		structureSpawnEnergy = layout.spawn.energy;
		structureSpawnEnergyCapacity = layout.spawn.energyCapacity;
	},

	writeGame(env, ptr) {
		let rooms = Object.keys(Game.rooms);
		let flushes = Array.apply(null, Array(rooms.length));
		let creepsBegin = ptr + gameCreeps + 4;
		let droppedResourcesBegin = ptr + gameDroppedResources + 4;
		let sourcesBegin = ptr + gameSources + 4;
		let structuresBegin = ptr + gameStructures + 4;
		let creepsCount = 0, droppedResourcesCount = 0, sourcesCount = 0, structuresCount = 0;

		// Write construction sites
		let constructionSiteIds = Object.keys(Game.constructionSites);
		constructionSiteIds.sort(function(left, right) {
			let leftRoom = Game.constructionSites[left].pos.roomName;
			let rightRoom = Game.constructionSites[right].pos.roomName;
			if (leftRoom === rightRoom) {
				return 0;
			} else if (leftRoom < rightRoom) {
				return -1;
			} else {
				return 1;
			}
		});
		let constructionSitesFirstByRoom = Object.create(null);
		let constructionSitesLastByRoom = Object.create(null);
		ArrayLib.write(env, ptr + gameConstructionSites, constructionSiteSizeof, 100, constructionSiteIds, function(env, offset, id) {
			let constructionSite = Game.constructionSites[id];
			let roomName = constructionSite.pos.roomName;
			if (constructionSitesFirstByRoom[roomName] === undefined) {
				constructionSitesFirstByRoom[roomName] = offset;
				constructionSitesLastByRoom[roomName] = offset + constructionSiteSizeof;
			} else {
				constructionSitesLastByRoom[roomName] = offset + constructionSiteSizeof;
			}
			that.writeConstructionSite(env, offset, constructionSite);
		});

		for (let ii = rooms.length - 1; ii >= 0; --ii) {
			// Get initial array state
			let roomName = rooms[ii];
			let room = Game.rooms[roomName];

			// Push room to arrays
			let creeps = room.find(FIND_CREEPS);
			let creepsEnd = ArrayLib.writeAppend(env, ptr + gameCreeps, creepSizeof, 100, creeps, that.writeCreep);
			creepsCount += creeps.length;
			let droppedResources = room.find(FIND_DROPPED_RESOURCES);
			let droppedResourcesEnd = ArrayLib.writeAppend(env, ptr + gameDroppedResources, droppedResourceSizeof, 100, droppedResources, that.writeDroppedResource);
			droppedResourcesCount += droppedResources.length;
			let sources = room.find(FIND_SOURCES);
			let sourcesEnd = ArrayLib.writeAppend(env, ptr + gameSources, sourceSizeof, 100, sources, that.writeSource);
			sourcesCount += sources.length;
			let structures = room.find(FIND_STRUCTURES);
			let structuresEnd = ArrayLib.writeAppend(env, ptr + gameStructures, structureSizeof, 100, structures, that.writeStructure);
			structuresCount += structures.length;
			let minerals = room.find(FIND_MINERALS);
			if (minerals.length >= 1) {
				if (minerals.length !== 1) {
					console.log('Found more than 1 mineral in room '+ room.roomName);
				}
				that.writeMineral(env, gameMineralHole, minerals[0]);
			}

			// Memoize arguments to flush all at once. I think this might improve v8 performance but
			// didn't actually test it.
			let roomLocation = env.screeps.position.parseRoomName(room.name);
			flushes[ii] = [
				ptr,
				roomLocation.rx, roomLocation.ry,
				room.energyAvailable, room.energyCapacityAvailable,
				minerals.length > 0 ? gameMineralHole : 0,
				constructionSitesFirstByRoom[roomName], constructionSitesLastByRoom[roomName],
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
		env.__ZN7screeps12game_state_t13reserve_roomsEPS0_j(ptr, flushes.length);
		for (let ii = flushes.length - 1; ii >= 0; --ii) {
			env.__ZN7screeps12game_state_t10flush_roomEPS0_jjjjPvS2_S2_S2_S2_S2_S2_S2_S2_S2_S2_S2_S2_.apply(env, flushes[ii]);
		}

		// Flush game
		env.HEAPU8[(ptr + gameGcl) >> 0] = Game.gcl;
		env.HEAPU32[(ptr + gameTime) >> 2] = Game.time;
		env.__ZN7screeps12game_state_t10flush_gameEPS0_(ptr);
	},

	writeRoomObject(env, ptr, pos) {
		let room = PositionLib.parseRoomName(pos.roomName);
		env.HEAPU16[(ptr + 0) >> 1] = pos.x + room.rx * 50;
		env.HEAPU16[(ptr + 2) >> 1] = pos.y + room.ry * 50;
	},

	writeGameObject(env, ptr, obj) {
		that.writeRoomObject(env, ptr, obj.pos);
		if (obj.id.length > 24) {
			throw new Error('`id` overflow');
		}
		StringLib.writeOneByteString(env, ptr + 4, obj.id);
	},

	writeConstructionSite(env, ptr, constructionSite) {
		that.writeGameObject(env, ptr, constructionSite);
		env.HEAPU8[(ptr + constructionSiteMy) >> 0] = constructionSite.my ? 1 : 0;
		env.HEAPU32[(ptr + constructionSiteProgress) >> 2] = constructionSite.progress;
		env.HEAPU32[(ptr + constructionSiteProgressTotal) >> 2] = constructionSite.progressTotal;
		env.HEAPU32[(ptr + constructionSiteType) >> 2] = structureTypeEnum.get(constructionSite.structureType);
	},

	writeCreep(env, ptr, creep) {
		that.writeGameObject(env, ptr, creep);
		that.writeResourceStore(env, ptr + creepCarry, creep.carry);
		env.HEAPU32[(ptr + creepCarryCapacity) >> 2] = creep.carryCapacity;
		env.HEAPU32[(ptr + creepFatigue) >> 2] = creep.fatigue;
		env.HEAPU32[(ptr + creepHits) >> 2] = creep.hits;
		env.HEAPU32[(ptr + creepHitsMax) >> 2] = creep.hitsMax;
		env.HEAP8[(ptr + creepMy) >> 0] = creep.my ? 1 : 0;
		if (creep.my) {
			StringLib.writeOneByteString(env, ptr + creepName, creep.name);
		} else {
			env.HEAPU32[(ptr + creepName) >> 2] = 0;
		}
		env.HEAP8[(ptr + creepSpawning) >> 0] = creep.spawning ? 1 : 0;
		env.HEAPU32[(ptr + creepTicksToLive) >> 2] = creep.ticksToLive;
	},

	writeDroppedResource(env, ptr, droppedResource) {
		that.writeGameObject(env, ptr, droppedResource);
		env.HEAPU32[(ptr + droppedResourceAmount) >> 2] = droppedResource.amount;
		env.HEAPU32[(ptr + droppedResourceType) >> 2] = resourceEnum.get(droppedResource.resourceType);
	},

	writeMineral(env, ptr, mineral) {
		that.writeGameObject(env, ptr, mineral);
		env.HEAPU32[(ptr + mineralType) >> 2] = resourceEnum.get(mineral.mineralType);
		env.HEAPU32[(ptr + mineralAmount) >> 2] = mineral.mineralAmount;
		env.HEAPU32[(ptr + mineralDensity) >> 2] = mineral.density;
		env.HEAPU32[(ptr + mineralTicksToRegeneration) >> 2] = mineral.ticksToRegeneration;
	},

	writeResourceStore(env, ptr, store) {
		let keys = Object.keys(store);
		if (keys.length == 0) {
			env.HEAPU32[ptr >> 2] = 0; // extended
			env.HEAPU32[(ptr + 8) >> 2] = 0; // single_amount
		} else if (keys.length == 1) {
			env.HEAPU32[ptr >> 2] = 0; // extended
			let type = keys[0];
			env.HEAPU32[(ptr + 4) >> 2] = resourceEnum.get(type) | 0; // single_type
			env.HEAPU32[(ptr + 8) >> 2] = store[type] | 0; // single_amount
		} else {
			let extended = ArrayLib.push(env, extendedResourceStorePtr, extendedResourceStoreSizeof, 100);
			env.HEAPU32.fill(extended >> 2, (extended + extendedResourceStoreSizeof) >> 2, 0);
			env.HEAPU32[ptr >> 2] = extended | 0;
			for (let ii = keys.length - 1; ii >= 0; --ii) {
				let key = keys[ii];
				env.HEAPU32[(extended + resourceEnum.get(key) * 4) >> 2] = store[key] | 0;
			}
		}
	},

	writeSource(env, ptr, source) {
		that.writeGameObject(env, ptr, source);
		env.HEAPU32[(ptr + sourceEnergy) >> 2] = source.energy;
		env.HEAPU32[(ptr + sourceEnergyCapacity) >> 2] = source.energyCapacity;
		env.HEAPU32[(ptr + sourceTicksToRegeneration) >> 2] = source.ticksToRegeneration;
	},

	writeStructure(env, ptr, structure) {
		that.writeGameObject(env, ptr, structure);
		env.HEAPU32[(ptr + structureStructureType) >> 2] = structureTypeEnum.get(structure.structureType);
		switch (structure.structureType) {
			case STRUCTURE_CONTROLLER:
			case STRUCTURE_KEEPER_LAIR:
			case STRUCTURE_PORTAL:
			case STRUCTURE_POWER_BANK:
				// indestrubable
			default:
				// Structure with hit points
				env.HEAPU32[(ptr + structureDestroyableHits) >> 2] = structure.hits;
				env.HEAPU32[(ptr + structureDestroyableHitsMax) >> 2] = structure.hitsMax;
				if (structure.structureType === STRUCTURE_ROAD) {
				} else if (structure.structureType === STRUCTURE_WALL) {
				} else {
					// Owned structure
					env.HEAPU8[(ptr + structureOwnedMy.my) >> 0] = structure.my;
					switch (structure.structureType) {
						case STRUCTURE_EXTENSION:
							env.HEAPU32[(ptr + structureExtensionEnergy) >> 2] = structure.energy;
							env.HEAPU32[(ptr + structureExtensionEnergyCapacity) >> 2] = structure.energyCapacity;
							break;
						case STRUCTURE_SPAWN:
							env.HEAPU32[(ptr + structureSpawnEnergy) >> 2] = structure.energy;
							env.HEAPU32[(ptr + structureSpawnEnergyCapacity) >> 2] = structure.energyCapacity;
							break;
					}
				}
				break;
		}
	},

	readCreepBodyPartArray(env, ptr) {
		return ArrayLib.map(env, ptr, 4, function(env, ptr) {
			return bodyPartEnumReverse.get(env.HEAPU32[ptr >> 2]);
		});
	},

	readResourceType(resourceType) {
		return resourceReverseEnum.get(resourceType);
	},
}
