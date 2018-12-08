#pragma once
#include "./constants.h"
#include "./object.h"
#include "./position.h"
#include "./resource.h"
#include "./structure.h"
#include <optional>

namespace screeps {

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

struct construction_site_t : public game_object_t {
	bool my;
	int32_t progress;
	int32_t progress_total;
	structure_t::type_t type;
};

struct dropped_resource_t : public game_object_t {
	resource_t type;
	int32_t amount;
};

struct flag_t : public game_object_t {
	string_t<kMaxFlagNameLength> name;
	color_t color;
	color_t secondary_color;
};

struct mineral_t : public game_object_t {
	resource_t type;
	int32_t amount;
	int32_t ticks_to_regeneration;
	int32_t density; // 1 - 4
};

struct source_t : public game_object_t {
	int32_t energy;
	int32_t energy_capacity;
	int32_t ticks_to_regeneration;
};

struct tombstone_t : public game_object_t {
	creep_t creep;
	int32_t death_time;
	resource_store_t store;
	int32_t ticks_to_decay;
};

class room_t {
	private:
		template <typename T> using list_t = pointer_container_t<T>;

	public:
		room_location_t location;
		int32_t energy_available;
		int32_t energy_capacity_available;
		controller_t* controller = nullptr;
		std::optional<mineral_t> mineral;
		// storage_t* storage = nullptr;
		// terminal_t* terminal = nullptr;

		list_t<construction_site_t> construction_sites;
		list_t<creep_t> creeps;
		list_t<dropped_resource_t> dropped_resources;
		// list_t<flag_t> flags;
		list_t<source_t> sources;
		list_t<structure_union_t> structures;
		list_t<tombstone_t> tombstones;

		room_t(
			room_location_t location,
			int32_t energy_available, int32_t energy_capacity_available,
			const mineral_t* mineral,
			construction_site_t* construction_sites_begin, construction_site_t* construction_sites_end,
			creep_t* creeps_begin, creep_t* creeps_end,
			dropped_resource_t* dropped_resources_begin, dropped_resource_t* dropped_resources_end,
			source_t* sources_begin, source_t* sources_end,
			structure_union_t* structures_begin, structure_union_t* structures_end,
			tombstone_t* tombstones_begin, tombstone_t* tombstones_end
		);

		template <class Memory>
		void serialize(Memory& memory) {
			memory & location;
			memory & energy_available & energy_capacity_available;
		}

		const terrain_t& terrain() const {
			return location.terrain();
		}

		static void init();
};

} // namespace screeps
