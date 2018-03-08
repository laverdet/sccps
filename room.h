#pragma once
#include "./constants.h"
#include "./object.h"
#include "./position.h"
#include "./resource.h"
#include "./structure.h"
#include <functional>
#include <unordered_map>
#include <vector>

enum class color_t {
	blue,
	brown,
	cyan,
	green,
	grey,
	orange,
	purple,
	red,
	white,
	yellow,
};

struct dropped_resource_t : public game_object_t {
	resource_t type;
	uint32_t amount;
};

struct flag_t : public game_object_t {
	string_t<kMaxFlagNameLength> name;
	color_t color;
	color_t secondary_color;
};

struct mineral_t : public game_object_t {
	resource_t type;
	uint32_t amount;
	uint8_t density; // 1 - 4
	uint32_t ticks_to_regeneration;
};

struct source_t : public game_object_t {
	uint32_t energy;
	uint32_t energy_capacity;
	uint32_t ticks_to_regeneration;
};

struct tombstone_t : public game_object_t {
	creep_t creep;
	uint32_t death_time;
	resource_store_t store;
	uint32_t ticks_to_decay;
};

class room_t {
	private:
		template <typename T> using list_t = std::vector<std::reference_wrapper<T>>;
		static std::unordered_map<room_location_t, terrain_t> terrain_map;

	public:
		room_location_t location;
		uint32_t energy_available;
		uint32_t energy_capacity_available;
		controller_t* controller = nullptr;
		mineral_t mineral;
		// storage_t* storage = nullptr;
		// terminal_t* terminal = nullptr;

		// construction_site
		// mineral
		list_t<creep_t> creeps;
		list_t<dropped_resource_t> dropped_resources;
		list_t<flag_t> flags;
		list_t<source_t> sources;
		list_t<structure_union_t> structures;
		list_t<tombstone_t> tombstones;

		room_t(
			room_location_t location,
			uint32_t energy_available, uint32_t energy_capacity_available,
			const mineral_t& mineral,
			creep_t* creeps_begin, creep_t* creeps_end,
			dropped_resource_t* dropped_resources_begin, dropped_resource_t* dropped_resources_end,
			source_t* sources_begin, source_t* sources_end,
			structure_union_t* structures_begin, structure_union_t* structures_end,
			tombstone_t* tombstones_begin, tombstone_t* tombstones_end
		) :
			location(location),
			energy_available(energy_available), energy_capacity_available(energy_capacity_available),
			mineral(mineral),
			creeps(creeps_begin, creeps_end),
			dropped_resources(dropped_resources_begin, dropped_resources_end),
			sources(sources_begin, sources_end),
			structures(structures_begin, structures_end),
			tombstones(tombstones_begin, tombstones_end) {}

		const terrain_t& get_terrain() const {
			return location.get_terrain();
		}

		static void init();
};
