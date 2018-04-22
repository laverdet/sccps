#pragma once
#include "./array.h"
#include "./constants.h"
#include "./creep.h"
#include "./object.h"
#include "./position.h"
#include "./room.h"
#include "./structure.h"
#include <cstdint>
#include <unordered_map>

namespace screeps {

/**
 * Game state
 */
class game_state_t {
	private:
		static void init();

		static void flush_game(
			game_state_t* game,
			uint32_t creeps_count,
			uint32_t dropped_resources_count,
			uint32_t sources_count,
			uint32_t structures_count,
			uint32_t tombstones_count
		);

		static void flush_room(
			game_state_t* game,
			uint32_t rx, uint32_t ry,
			uint32_t energy_available, uint32_t energy_capacity_available,
			void* mineral_ptr,
			void* creeps_begin, void* creeps_end,
			void* dropped_resources_begin, void* dropped_resources_end,
			void* sources_begin, void* sources_end,
			void* structure_begin, void* structure_end,
			void* tombstones_begin, void* tombstones_end
		);

		static void reserve_rooms(game_state_t* game, uint32_t rooms_count);

	public:
		uint8_t gcl;
		uint32_t time;

		std::unordered_map<room_location_t, room_t> rooms;
		std::unordered_map<id_t, creep_t*> creeps_by_id;
		std::unordered_map<creep_t::name_t, creep_t*> creeps_by_name;
		std::unordered_map<id_t, dropped_resource_t*> dropped_resources_by_id;
		std::unordered_map<id_t, source_t*> sources_by_id;
		std::unordered_map<id_t, structure_union_t*> structures_by_id;
		std::unordered_map<id_t, tombstone_t*> tombstones_by_id;

		array_t<creep_t, kMaximumCreeps> creeps;
		array_t<dropped_resource_t, 100> dropped_resources;
		array_t<flag_t, 100> flags;
		array_t<source_t, 100> sources;
		array_t<structure_union_t, kMaximumStructures> structures;
		array_t<tombstone_t, 100> tombstones;

		void load();
		creep_t* creep_by_name(const creep_t::name_t& name);
		const creep_t* creep_by_name(const creep_t::name_t& name) const;
		dropped_resource_t* dropped_resource_by_id(const id_t& id);
		const dropped_resource_t* dropped_resource_by_id(const id_t& id) const;
		source_t* source_by_id(const id_t& id);
		const source_t* source_by_id(const id_t& id) const;
		structure_union_t* structure_by_id(const id_t& id);
		const structure_union_t* structure_by_id(const id_t& id) const;
};

void* exception_what(void* ptr);

} // namespace screeps
