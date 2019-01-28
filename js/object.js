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
let creepPtr; // hack to write spawning creeps
let creepSizeof;
let creepBody, creepCarry, creepFatigue, creepHits, creepHitsMax, creepMy, creepName, creepIsSpawning, creepSpawnId, creepTicksToLive;
let creepBodyPartSizeof, creepBodyPartBoost, creepBodyPartType;

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
	undefined,
]);
let flagSizeof;
let flagName, flagColor, flagSecondaryColor;

// mineral_t
let mineralSizeof;
let mineralAmount, mineralDensity, mineralType, mineralTicksToRegeneration;

// room_t
let roomLocation;
let roomConstructionSites, roomCreeps, roomDroppedResources, roomSources, roomStructures, roomTombstones;
let roomEnergyAvailable, roomEnergyCapacityAvailable;
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
let structureStructureType, structureHits, structureHitsMax, structureOwner, structureMy;
let structureContainerStore, structureContainerTicksToDecay;
let structureControllerLevel, structureControllerProgress, structureControllerProgressTotal, structureControllerTicksToDowngrade, structureControllerUpgradeBlocked;
let structureExtensionEnergy, structureExtensionEnergyCapacity;
let structureRoadTicksToDecay;
let structureSpawnEnergy, structureSpawnEnergyCapacity;
let structureSpawning, structureSpawningDirections, structureSpawningNeedTime, structureSpawningRemainingTime, structureSpawningId;

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
		creepFatigue = layout.fatigue;
		creepHits = layout.hits;
		creepHitsMax = layout.hitsMax;
		creepMy = layout.my;
		creepName = layout.name;
		creepIsSpawning = layout.isSpawning;
		creepSpawnId = layout.spawnId;
		creepTicksToLive = layout.ticksToLive;
		creepBodyPartSizeof = layout.bodyPartSizeof;
		creepBodyPartBoost = layout.bodyPartBoost;
		creepBodyPartType = layout.bodyPartType;
	},

	initFlagLayout(layout) {
		flagSizeof = layout.sizeof;
		flagName = layout.name;
		flagColor = layout.color;
		flagSecondaryColor = layout.secondaryColor;
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
		roomEnergyAvailable = layout.energyAvailable;
		roomEnergyCapacityAvailable = layout.energyCapacityAvailable;
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
		structureStructureType = layout.structureType;
		structureHits = layout.hits;
		structureHitsMax = layout.hitsMax;
		structureOwner = layout.owner;
		structureMy = layout.my;
	},

	initStructureContainerLayout(layout) {
		structureContainerStore = layout.store;
		structureContainerTicksToDecay = layout.ticksToDecay;
	},

	initStructureControllerLayout(layout) {
		structureControllerLevel = layout.level;
		structureControllerProgress = layout.progress;
		structureControllerProgressTotal = layout.progressTotal;
		structureControllerTicksToDowngrade = layout.ticksToDowngrade;
		structureControllerUpgradeBlocked = layout.upgradeBlocked;
	},

	initStructureExtensionLayout(layout) {
		structureExtensionEnergy = layout.energy;
		structureExtensionEnergyCapacity = layout.energyCapacity;
	},

	initStructureRoadLayout(layout) {
		structureRoadTicksToDecay = layout.ticksToDecay;
	},

	initStructureSpawnLayout(layout) {
		structureSpawnEnergy = layout.energy;
		structureSpawnEnergyCapacity = layout.energyCapacity;
		structureSpawning = layout.spawning;
		structureSpawningDirections = layout.spawningDirections;
		structureSpawningNeedTime = layout.spawningNeedTime;
		structureSpawningRemainingTime = layout.spawningRemainingTime;
		structureSpawningId = layout.spawningId;
	},

	writeGame(env, ptr) {

		// Write game data
		// env.writeInt32(ptr + gameGcl, Game.gcl);
		env.writeInt32(ptr + gameTime, Game.time);

		// Ensure vector capacity
		let constructionSites = Object.values(Game.constructionSites);
		let flags = Object.values(Game.flags);
		let rooms = Object.values(Game.rooms);
		let roomPointersLength = Math.max(env.readUint32(ptr + gameRooms), rooms.length);
		let needsResize =
			env.readUint32(ptr + gameConstructionSites) < constructionSites.length ||
			env.readUint32(ptr + gameFlags) < flags.length ||
			env.readUint32(ptr + gameRooms) < rooms.length;
		env.writeUint32(ptr + gameConstructionSites, constructionSites.length);
		env.writeUint32(ptr + gameFlags, flags.length);
		env.writeUint32(ptr + gameRooms, rooms.length);
		if (needsResize) {
			env.__ZN7screeps12game_state_t15ensure_capacityEPS0_(ptr);
		}

		// Write sites and flags
		constructionSites.sort(function(left, right) {
			return PositionLib.parseRoomName(left.pos.roomName) - PositionLib.parseRoomName(right.pos.roomName);
		});
		flags.sort(function(left, right) {
			return PositionLib.parseRoomName(left.pos.roomName) - PositionLib.parseRoomName(right.pos.roomName);
		});
		ArrayLib.writeData(env, env.readPtr(ptr + gameConstructionSites + env.ptrSize), constructionSiteSizeof, constructionSites, that.writeConstructionSite);
		ArrayLib.writeData(env, env.readPtr(ptr + gameFlags + env.ptrSize), flagSizeof, flags, that.writeFlag);

		// Write rooms
		let roomPointers = env.readPtr(ptr + gameRooms + env.ptrSize);
		let roomPointersEnd = roomPointers + roomPointersLength * env.ptrSize;
		let extraRoomPointers = roomPointers;
		let unusedRoomPointers = [];
		while (roomPointers !== roomPointersEnd && env.readUint32(env.readPtr(roomPointers) + roomLocation) === 0) {
			roomPointers += env.ptrSize;
		}
		rooms.sort(function(left, right) {
			return PositionLib.parseRoomName(left.name) - PositionLib.parseRoomName(right.name);
		});
		for (let room of rooms) {
			// Find pointer to room structure
			let roomId = PositionLib.parseRoomName(room.name);
			let roomPtr;
			do {
				if (roomPointers === roomPointersEnd) {
					if (unusedRoomPointers.length === 0) {
						roomPtr = env.readPtr(extraRoomPointers);
						extraRoomPointers += env.ptrSize;
					} else {
						roomPtr = unusedRoomPointers.pop();
					}
					break;
				} else {
					roomPtr = env.readPtr(roomPointers);
					let nextRoomId = env.readInt32(roomPtr + roomLocation);
					if (nextRoomId < roomId) {
						roomPointers += env.ptrSize;
						unusedRoomPointers.push(roomPtr);
						continue;
					} else if (nextRoomId > roomId) {
						roomPtr = env.readPtr(extraRoomPointers);
						extraRoomPointers += env.ptrSize;
					}
					break;
				}
			} while (true);
			that.writeRoom(env, roomPtr, roomId, room);
		}
	},

	writeRoom(env, ptr, roomId, room) {
		// Write room data
		env.writeInt32(ptr + roomLocation, roomId);
		env.writeInt32(ptr + roomEnergyAvailable, room.energyAvailable);
		env.writeInt32(ptr + roomEnergyCapacityAvailable, room.energyCapacityAvailable);

		// Ensure vector capacity
		let creeps = room.find(FIND_CREEPS);
		let droppedResources = room.find(FIND_DROPPED_RESOURCES);
		let sources = room.find(FIND_SOURCES);
		let structures = room.find(FIND_STRUCTURES);
		let needsResize =
			env.readUint32(ptr + roomCreeps) < creeps.length + 3 ||
			env.readUint32(ptr + roomDroppedResources) < droppedResources.length ||
			env.readUint32(ptr + roomSources) < sources.length ||
			env.readUint32(ptr + roomStructures) < structures.length;
		env.writeUint32(ptr + roomCreeps, creeps.length + 3);
		env.writeUint32(ptr + roomDroppedResources, droppedResources.length);
		env.writeUint32(ptr + roomSources, sources.length);
		env.writeUint32(ptr + roomStructures, structures.length);
		if (needsResize) {
			env.__ZN7screeps6room_t15ensure_capacityEPS0_(ptr);
		}

		// Write structures
		let creepsData = env.readPtr(ptr + roomCreeps + env.ptrSize);
		creepPtr = creepsData + creepSizeof * creeps.length;
		ArrayLib.writeData(env, creepsData, creepSizeof, creeps, that.writeCreep);
		ArrayLib.writeData(env, env.readPtr(ptr + roomDroppedResources + env.ptrSize), droppedResourceSizeof, droppedResources, that.writeDroppedResource);
		ArrayLib.writeData(env, env.readPtr(ptr + roomSources + env.ptrSize), sourceSizeof, sources, that.writeSource);
		ArrayLib.writeData(env, env.readPtr(ptr + roomStructures + env.ptrSize), structureSizeof, structures, that.writeStructure);

		// Update creeps length now that we've searched all spawns for extra creeps
		env.writeUint32(ptr + roomCreeps, creeps.length + (creepPtr - creepsData) / creepSizeof);

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
		env.writePtr(ptr + roomMineral, mineralPtr);
	},

	writeRoomObject(env, ptr, pos) {
		let room = PositionLib.parseRoomName(pos.roomName);
		env.writeUint16(ptr + 0, pos.x + (room >> 16) * 50);
		env.writeUint16(ptr + 2, pos.y + (room & 0xff) * 50);
	},

	writeGameObject(env, ptr, obj) {
		that.writeRoomObject(env, ptr, obj.pos);
		if (obj.id.length > 24) {
			throw new Error('`id` overflow');
		}
		StringLib.writeOneByteString(env, ptr + env.ptrSize, obj.id);
	},

	writeConstructionSite(env, ptr, constructionSite) {
		that.writeGameObject(env, ptr, constructionSite);
		env.writeInt8(ptr + constructionSiteMy, constructionSite.my ? 1 : 0);
		env.writeInt32(ptr + constructionSiteProgress, constructionSite.progress);
		env.writeInt32(ptr + constructionSiteProgressTotal, constructionSite.progressTotal);
		env.writeInt32(ptr + constructionSiteType, structureTypeEnum.get(constructionSite.structureType));
	},

	writeCreep(env, ptr, creep) {
		that.writeGameObject(env, ptr, creep);
		that.writeResourceStore(env, ptr + creepCarry, creep.carry, creep.carryCapacity);
		ArrayLib.write(env, ptr + creepBody, creepBodyPartSizeof, 50, creep.body, function(env, ptr, part) {
			env.writeInt32(ptr + creepBodyPartBoost, resourceEnum.get(part.boost));
			env.writeInt32(ptr + creepBodyPartType, bodyPartEnum.get(part.type));
		});
		env.writeInt32(ptr + creepFatigue, creep.fatigue);
		env.writeInt32(ptr + creepHits, creep.hits);
		env.writeInt32(ptr + creepHitsMax, creep.hitsMax);
		env.writeInt8(ptr + creepMy, creep.my ? 1 : 0);
		if (creep.my) {
			StringLib.writeOneByteString(env, ptr + creepName, creep.name);
		} else {
			env.writeInt32(ptr + creepName, 0);
		}
		env.writeInt8(ptr + creepIsSpawning, creep.spawning ? 1 : 0);
		env.writeInt32(ptr + creepTicksToLive, creep.ticksToLive);
	},

	writeDroppedResource(env, ptr, droppedResource) {
		that.writeGameObject(env, ptr, droppedResource);
		env.writeInt32(ptr + droppedResourceAmount, droppedResource.amount);
		env.writeInt32(ptr + droppedResourceType, resourceEnum.get(droppedResource.resourceType));
	},

	writeFlag(env, ptr, flag) {
		that.writeRoomObject(env, ptr, flag.pos);
		StringLib.writeOneByteString(env, ptr + flagName, flag.name);
		env.writeInt32(ptr + flagColor, flag.color);
		env.writeInt32(ptr + flagSecondaryColor, flag.secondaryColor);
	},

	writeMineral(env, ptr, mineral) {
		that.writeGameObject(env, ptr, mineral);
		env.writeInt32(ptr + mineralType, resourceEnum.get(mineral.mineralType));
		env.writeInt32(ptr + mineralAmount, mineral.mineralAmount);
		env.writeInt32(ptr + mineralDensity, mineral.density);
		env.writeInt32(ptr + mineralTicksToRegeneration, mineral.ticksToRegeneration);
	},

	writeResourceStore(env, ptr, store, capacity) {
		let keys = Object.keys(store);
		if (keys.length == 0) {
			env.writeInt32(ptr + 8, 0); // single_amount
		} else if (keys.length == 1) {
			let type = keys[0];
			env.writeInt32(ptr + 4, resourceEnum.get(type)); // single_type
			env.writeInt32(ptr + 8, store[type]); // single_amount
		} else {
			throw new Error('More than one resource');
		}
		env.writeInt32(ptr, capacity); // capacity
	},

	writeSource(env, ptr, source) {
		that.writeGameObject(env, ptr, source);
		env.writeInt32(ptr + sourceEnergy, source.energy);
		env.writeInt32(ptr + sourceEnergyCapacity, source.energyCapacity);
		env.writeInt32(ptr + sourceTicksToRegeneration, source.ticksToRegeneration);
	},

	writeStructure(env, ptr, structure) {
		that.writeGameObject(env, ptr, structure);
		env.writeInt32(ptr + structureStructureType, structureTypeEnum.get(structure.structureType));
		env.writeInt32(ptr + structureHits, structure.hits);
		env.writeInt32(ptr + structureHitsMax, structure.hitsMax);
		// TODO: owners
		env.writeInt8(ptr + structureMy, structure.my);
		switch (structure.structureType) {
			case STRUCTURE_CONTAINER:
				that.writeResourceStore(env, ptr + structureContainerStore, structure.store, structure.storeCapacity);
				env.writeInt32(ptr + structureContainerTicksToDecay, structure.ticksToDecay);
				break;

			case STRUCTURE_CONTROLLER:
				env.writeInt32(ptr + structureControllerLevel, structure.level);
				env.writeInt32(ptr + structureControllerProgress, structure.progress);
				env.writeInt32(ptr + structureControllerProgressTotal, structure.progressTotal);
				env.writeInt32(ptr + structureControllerTicksToDowngrade, structure.ticksToDowngrade);
				env.writeInt32(ptr + structureControllerUpgradeBlocked, structure.upgradeBlocked);
				break;

			case STRUCTURE_EXTENSION:
				env.writeInt32(ptr + structureExtensionEnergy, structure.energy);
				env.writeInt32(ptr + structureExtensionEnergyCapacity, structure.energyCapacity);
				break;

			case STRUCTURE_ROAD:
				env.writeInt32(ptr + structureRoadTicksToDecay, structure.ticksToDecay);
				break;

			case STRUCTURE_SPAWN:
				env.writeInt32(ptr + structureSpawnEnergy, structure.energy);
				env.writeInt32(ptr + structureSpawnEnergyCapacity, structure.energyCapacity);
				if (structure.spawning) {
					let bits = 0;
					if (structure.spawning.directions !== undefined) {
						let directions = structure.spawning.directions;
						for (let ii = directions.length - 1; ii <= 0; --ii) {
							bits <<= 4;
							bits |= directions[ii];
						}
					}
					env.writeInt32(ptr + structureSpawningDirections, bits);
					env.writeInt32(ptr + structureSpawningNeedTime, structure.spawning.needTime);
					env.writeInt32(ptr + structureSpawningRemainingTime, structure.spawning.remainingTime);
					let creep = Game.creeps[structure.spawning.name];
					StringLib.writeOneByteString(env, ptr + structureSpawningId, creep.id);
					env.writeInt8(ptr + structureSpawning, 1);
					// The spawn is responsible for writing creep data because these creeps don't show up in Game.creeps
					that.writeCreep(env, creepPtr, creep);
					StringLib.writeOneByteString(env, creepPtr + creepSpawnId, structure.id);
					creepPtr += creepSizeof;
				} else {
					env.writeInt8(ptr + structureSpawning, 0);
				}
				break;
		}
	},

	readColor(color) {
		return colorEnumReverse.get(color);
	},

	readCreepBodyPart(bodyPart) {
		return bodyPartEnumReverse.get(bodyPart);
	},

	readResourceType(resourceType) {
		return resourceReverseEnum.get(resourceType);
	},

	readStructureType(structureType) {
		return structureTypeEnumReverse.get(structureType);
	},
}
