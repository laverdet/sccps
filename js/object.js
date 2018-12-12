'use strict';
const ArrayLib = require('array');
const PositionLib = require('position');
const StringLib = require('string');
const util = require('util');
const kWorldSize = 255;

// game_state_t
let gameConstructionSites, gameFlags, gameRooms;
let gameGcl, gameTime;

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
	undefined,
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
let creepBody, creepCarry, creepCarryCapacity, creepFatigue, creepHits, creepHitsMax, creepMy, creepName, creepSpawning, creepTicksToLive;
let creepBodyPartSizeof, creepBodyPartBoost, creepBodyPartType;

// dropped_resource_t
let droppedResourceSizeof;
let droppedResourceAmount, droppedResourceType;

// flag_t
const [ colorEnum, colorEnumReverse ] = util.enumToMap([
	undefined,
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

// room_t
let roomLocation;
let roomConstructionSites, roomCreeps, roomDroppedResources, roomSources, roomStructures, roomTombstones;
let roomMineral, roomMineralHolder;

// source_t
let sourceSizeof;
let sourceEnergy, sourceEnergyCapacity, sourceTicksToRegeneration;

// structure_t
const [ structureTypeEnum, structureTypeEnumReverse ] = util.enumToMap([
	undefined,
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
let structureControllerLevel, structureControllerProgress, structureControllerProgressTotal, structureControllerTicksToDowngrade, structureControllerUpgradeBlocked;
let structureExtensionEnergy, structureExtensionEnergyCapacity;
let structureSpawnEnergy, structureSpawnEnergyCapacity;

const that = module.exports = {
	initGameLayout(layout) {
		gameConstructionSites = layout.constructionSites;
		gameRooms = layout.rooms;
		gameFlags = layout.flags;

		gameGcl = layout.gcl;
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
		creepBody = layout.body;
		creepCarry = layout.carry;
		creepCarryCapacity = layout.carryCapacity;
		creepFatigue = layout.fatigue;
		creepHits = layout.hits;
		creepHitsMax = layout.hitsMax;
		creepMy = layout.my;
		creepName = layout.name;
		creepSpawning = layout.spawning;
		creepBodyPartSizeof = layout.bodyPartSizeof;
		creepBodyPartBoost = layout.bodyPartBoost;
		creepBodyPartType = layout.bodyPartType;
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

	initRoomLayout(layout) {
		roomLocation = layout.location;
		roomCreeps = layout.creeps;
		roomDroppedResources = layout.droppedResources;
		roomSources = layout.sources;
		roomStructures = layout.structures;
		roomTombstones = layout.tombstones;
		roomMineral = layout.mineral;
		roomMineralHolder = layout.mineralHolder;
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

		structureControllerLevel = layout.controller.level;
		structureControllerProgress = layout.controller.progress;
		structureControllerProgressTotal = layout.controller.progressTotal;
		structureControllerTicksToDowngrade = layout.controller.ticksToDowngrade;
		structureControllerUpgradeBlocked = layout.controller.upgradeBlocked;

		structureExtensionEnergy = layout.extension.energy;
		structureExtensionEnergyCapacity = layout.extension.energyCapacity;

		structureSpawnEnergy = layout.spawn.energy;
		structureSpawnEnergyCapacity = layout.spawn.energyCapacity;
	},

	writeGame(env, ptr) {
		// Write game data
		env.HEAP32[(ptr + gameGcl) >> 2] = Game.gcl;
		env.HEAP32[(ptr + gameTime) >> 2] = Game.time;

		// Ensure vector capacity
		let constructionSites = Object.values(Game.constructionSites);
		let flags = Object.values(Game.flags);
		let rooms = Object.keys(Game.rooms);
		let roomPointersLength = Math.max(env.HEAPU32[((ptr + gameRooms) >> 2) + 1], rooms.length);
		let needsResize =
			env.HEAPU32[((ptr + gameConstructionSites) >> 2) + 1] < constructionSites.length ||
			env.HEAPU32[((ptr + gameFlags) >> 2) + 1] < flags.length ||
			env.HEAPU32[((ptr + gameRooms) >> 2) + 1] < rooms.length;
		env.HEAPU32[((ptr + gameConstructionSites) >> 2) + 1] = constructionSites.length;
		env.HEAPU32[((ptr + gameFlags) >> 2) + 1] = flags.length;
		env.HEAPU32[((ptr + gameRooms) >> 2) + 1] = rooms.length;
		if (needsResize) {
			env.__ZN7screeps12game_state_t15ensure_capacityEPS0_(ptr);
		}

		// Write sites and flags
		constructionSites.sort(function(left, right) {
			return PositionLib.parseRoomName(left.pos.roomName) - PositionLib.parseRoomName(right.pos.roomName);
		});
		ArrayLib.writeData(env, env.HEAPU32[(ptr + gameConstructionSites) >> 2], constructionSiteSizeof, constructionSites, that.writeConstructionSite);

		// Write rooms
		let roomPointers = env.HEAPU32[(ptr + gameRooms) >> 2] >> 2;
		let roomPointersEnd = roomPointers + roomPointersLength;
		let extraRoomPointers = roomPointers;
		let unusedRoomPointers = [];
		while (roomPointers !== roomPointersEnd && env.HEAPU32[(env.HEAPU32[roomPointers] + roomLocation) >> 2] === 0) {
			++roomPointers;
		}
		rooms.sort(function(left, right) {
			return PositionLib.parseRoomName(left) - PositionLib.parseRoomName(right);
		});
		for (let roomName of rooms) {
			// Find pointer to room structure
			let roomId = PositionLib.parseRoomName(roomName);
			let roomPtr;
			do {
				if (roomPointers === roomPointersEnd) {
					if (unusedRoomPointers.length === 0) {
						roomPtr = env.HEAPU32[extraRoomPointers++];
					} else {
						roomPtr = unusedRoomPointers.pop();
					}
					break;
				} else {
					roomPtr = env.HEAPU32[roomPointers];
					let nextRoomId = env.HEAP32[(roomPtr + roomLocation) >> 2];
					if (nextRoomId < roomId) {
						++roomPointers;
						unusedRoomPointers.push(roomPtr);
						continue;
					} else if (nextRoomId > roomId) {
						roomPtr = env.HEAPU32[extraRoomPointers++];
					}
					break;
				}
			} while (true);
			that.writeRoom(env, roomPtr, roomId, Game.rooms[roomName]);
		}
	},

	writeRoom(env, ptr, roomId, room) {
		// Write room data
		env.HEAP32[(ptr + roomLocation) >> 2] = roomId;

		// Ensure vector capacity
		let creeps = room.find(FIND_CREEPS);
		let droppedResources = room.find(FIND_DROPPED_RESOURCES);
		let sources = room.find(FIND_SOURCES);
		let structures = room.find(FIND_STRUCTURES);
		let needsResize =
			env.HEAPU32[((ptr + roomCreeps) >> 2) + 1] < creeps.length ||
			env.HEAPU32[((ptr + roomDroppedResources) >> 2) + 1] < droppedResources.length ||
			env.HEAPU32[((ptr + roomSources) >> 2) + 1] < sources.length ||
			env.HEAPU32[((ptr + roomStructures) >> 2) + 1] < structures.length;
		env.HEAPU32[((ptr + roomCreeps) >> 2) + 1] = creeps.length;
		env.HEAPU32[((ptr + roomDroppedResources) >> 2) + 1] = droppedResources.length;
		env.HEAPU32[((ptr + roomSources) >> 2) + 1] = sources.length;
		env.HEAPU32[((ptr + roomStructures) >> 2) + 1] = structures.length;
		if (needsResize) {
			env.__ZN7screeps6room_t15ensure_capacityEPS0_(ptr);
		}

		// Write structures
		ArrayLib.writeData(env, env.HEAPU32[(ptr + roomCreeps) >> 2], creepSizeof, creeps, that.writeCreep);
		ArrayLib.writeData(env, env.HEAPU32[(ptr + roomDroppedResources) >> 2], droppedResourceSizeof, droppedResources, that.writeDroppedResource);
		ArrayLib.writeData(env, env.HEAPU32[(ptr + roomSources) >> 2], sourceSizeof, sources, that.writeSource);
		ArrayLib.writeData(env, env.HEAPU32[(ptr + roomStructures) >> 2], structureSizeof, structures, that.writeStructure);

		// Write mineral
		let minerals = room.find(FIND_MINERALS);
		let mineralPtr = 0;
		if (minerals.length >= 1) {
			if (minerals.length !== 1) {
				console.log(`Found more than 1 mineral in room ${room.roomName}`);
			}
			mineralPtr = ptr + roomMineralHolder;
			that.writeMineral(env, mineralPtr, minerals[0]);
		}
		env.HEAPU32[(ptr + roomMineral) >> 2] = mineralPtr;
	},

	writeRoomObject(env, ptr, pos) {
		let room = PositionLib.parseRoomName(pos.roomName);
		env.HEAPU16[(ptr + 0) >> 1] = pos.x + (room >> 16) * 50;
		env.HEAPU16[(ptr + 2) >> 1] = pos.y + (room & 0xff) * 50;
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
		env.HEAP8[(ptr + constructionSiteMy) >> 0] = constructionSite.my ? 1 : 0;
		env.HEAP32[(ptr + constructionSiteProgress) >> 2] = constructionSite.progress;
		env.HEAP32[(ptr + constructionSiteProgressTotal) >> 2] = constructionSite.progressTotal;
		env.HEAP32[(ptr + constructionSiteType) >> 2] = structureTypeEnum.get(constructionSite.structureType);
	},

	writeCreep(env, ptr, creep) {
		that.writeGameObject(env, ptr, creep);
		that.writeResourceStore(env, ptr + creepCarry, creep.carry);
		ArrayLib.write(env, ptr + creepBody, creepBodyPartSizeof, 50, creep.body, function(env, ptr, part) {
			env.HEAP32[(ptr + creepBodyPartBoost) >> 2] = resourceEnum.get(part.boost);
			env.HEAP32[(ptr + creepBodyPartType) >> 2] = bodyPartEnum.get(part.type);
		});
		env.HEAP32[(ptr + creepCarryCapacity) >> 2] = creep.carryCapacity;
		env.HEAP32[(ptr + creepFatigue) >> 2] = creep.fatigue;
		env.HEAP32[(ptr + creepHits) >> 2] = creep.hits;
		env.HEAP32[(ptr + creepHitsMax) >> 2] = creep.hitsMax;
		env.HEAP8[(ptr + creepMy) >> 0] = creep.my ? 1 : 0;
		if (creep.my) {
			StringLib.writeOneByteString(env, ptr + creepName, creep.name);
		} else {
			env.HEAP32[(ptr + creepName) >> 2] = 0;
		}
		env.HEAP8[(ptr + creepSpawning) >> 0] = creep.spawning ? 1 : 0;
		env.HEAP32[(ptr + creepTicksToLive) >> 2] = creep.ticksToLive;
	},

	writeDroppedResource(env, ptr, droppedResource) {
		that.writeGameObject(env, ptr, droppedResource);
		env.HEAP32[(ptr + droppedResourceAmount) >> 2] = droppedResource.amount;
		env.HEAP32[(ptr + droppedResourceType) >> 2] = resourceEnum.get(droppedResource.resourceType);
	},

	writeMineral(env, ptr, mineral) {
		that.writeGameObject(env, ptr, mineral);
		env.HEAP32[(ptr + mineralType) >> 2] = resourceEnum.get(mineral.mineralType);
		env.HEAP32[(ptr + mineralAmount) >> 2] = mineral.mineralAmount;
		env.HEAP32[(ptr + mineralDensity) >> 2] = mineral.density;
		env.HEAP32[(ptr + mineralTicksToRegeneration) >> 2] = mineral.ticksToRegeneration;
	},

	writeResourceStore(env, ptr, store) {
		let keys = Object.keys(store);
		if (keys.length == 0) {
			env.HEAP32[ptr >> 2] = 0; // extended
			env.HEAP32[(ptr + 8) >> 2] = 0; // single_amount
		} else if (keys.length == 1) {
			env.HEAP32[ptr >> 2] = 0; // extended
			let type = keys[0];
			env.HEAP32[(ptr + 4) >> 2] = resourceEnum.get(type) | 0; // single_type
			env.HEAP32[(ptr + 8) >> 2] = store[type] | 0; // single_amount
		} else {
			let extended = ArrayLib.push(env, extendedResourceStorePtr, extendedResourceStoreSizeof, 100);
			env.HEAP32.fill(extended >> 2, (extended + extendedResourceStoreSizeof) >> 2, 0);
			env.HEAP32[ptr >> 2] = extended | 0;
			for (let ii = keys.length - 1; ii >= 0; --ii) {
				let key = keys[ii];
				env.HEAP32[(extended + resourceEnum.get(key) * 4) >> 2] = store[key] | 0;
			}
		}
	},

	writeSource(env, ptr, source) {
		that.writeGameObject(env, ptr, source);
		env.HEAP32[(ptr + sourceEnergy) >> 2] = source.energy;
		env.HEAP32[(ptr + sourceEnergyCapacity) >> 2] = source.energyCapacity;
		env.HEAP32[(ptr + sourceTicksToRegeneration) >> 2] = source.ticksToRegeneration;
	},

	writeStructure(env, ptr, structure) {
		that.writeGameObject(env, ptr, structure);
		env.HEAP32[(ptr + structureStructureType) >> 2] = structureTypeEnum.get(structure.structureType);
		switch (structure.structureType) {
			case STRUCTURE_CONTROLLER:
				env.HEAP32[(ptr + structureControllerLevel) >> 2] = structure.level;
				env.HEAP32[(ptr + structureControllerProgress) >> 2] = structure.progress;
				env.HEAP32[(ptr + structureControllerProgressTotal) >> 2] = structure.progressTotal;
				env.HEAP32[(ptr + structureControllerTicksToDowngrade) >> 2] = structure.ticksToDowngrade;
				env.HEAP32[(ptr + structureControllerUpgradeBlocked) >> 2] = structure.upgradeBlocked;
				break;

			case STRUCTURE_KEEPER_LAIR:
			case STRUCTURE_PORTAL:
			case STRUCTURE_POWER_BANK:
				// indestrubable
				break;
			default:
				// Structure with hit points
				env.HEAP32[(ptr + structureDestroyableHits) >> 2] = structure.hits;
				env.HEAP32[(ptr + structureDestroyableHitsMax) >> 2] = structure.hitsMax;
				if (structure.structureType === STRUCTURE_ROAD) {
				} else if (structure.structureType === STRUCTURE_WALL) {
				} else {
					// Owned structure
					env.HEAP8[(ptr + structureOwnedMy.my) >> 0] = structure.my;
					switch (structure.structureType) {
						case STRUCTURE_EXTENSION:
							env.HEAP32[(ptr + structureExtensionEnergy) >> 2] = structure.energy;
							env.HEAP32[(ptr + structureExtensionEnergyCapacity) >> 2] = structure.energyCapacity;
							break;
						case STRUCTURE_SPAWN:
							env.HEAP32[(ptr + structureSpawnEnergy) >> 2] = structure.energy;
							env.HEAP32[(ptr + structureSpawnEnergyCapacity) >> 2] = structure.energyCapacity;
							break;
					}
				}
				break;
		}
	},

	readCreepBodyPartArray(env, ptr) {
		return ArrayLib.map(env, ptr, 4, function(env, ptr) {
			return bodyPartEnumReverse.get(env.HEAP32[ptr >> 2]);
		});
	},

	readResourceType(resourceType) {
		return resourceReverseEnum.get(resourceType);
	},

	readStructureType(structureType) {
		return structureTypeEnumReverse.get(structureType);
	},
}
