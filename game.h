#pragma once
#include <cstdint>

#include "./array.h"
#include "./string.h"

#include "./constants.h"

#include "./creep.h"
#include "./object.h"
#include "./path-finder.h"
#include "./position.h"
#include "./resource.h"
#include "./room.h"
#include "./structure.h"
#include "./terrain.h"

#include <unordered_map>

/**
 * Game state
 */
struct game_state_t {
	std::unordered_map<room_location_t, room_t> rooms;

	array_t<creep_t, kMaximumCreeps> creeps;
	array_t<dropped_resource_t, 100> dropped_resources;
	array_t<flag_t, 100> flags;
	array_t<source_t, 100> sources;
	array_t<structure_union_t, kMaximumStructures> structures;
	array_t<tombstone_t, 100> tombstones;

	void clear();
	void load();

	static void flush_room(
		game_state_t* game,
		uint32_t rx, uint32_t ry,
		uint32_t energy_available, uint32_t energy_capacity_available,
		void* creeps_begin, void* creeps_end,
		void* dropped_resources_begin, void* dropped_resources_end,
		void* sources_begin, void* sources_end,
		void* structure_begin, void* structure_end,
		void* tombstones_begin, void* tombstones_end
	);
};
