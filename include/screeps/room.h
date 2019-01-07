#pragma once
#include "./constants.h"
#include "./object.h"
#include "./iterator.h"
#include "./position.h"
#include "./resource.h"
#include "./structure.h"
#include "./internal/memory.h"

namespace screeps {

enum class color_t {
	none,
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

	template <class Memory>
	void serialize(Memory& memory) {
		game_object_t::serialize(memory);
		memory & my & progress & progress_total & type;
	}
};

struct dropped_resource_t : public game_object_t {
	resource_t type;
	int32_t amount;

	template <class Memory>
	void serialize(Memory& memory) {
		game_object_t::serialize(memory);
		memory & type & amount;
	}
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

	template <class Memory>
	void serialize(Memory& memory) {
		game_object_t::serialize(memory);
		memory & type & amount & ticks_to_regeneration & density;
	}
};

struct source_t : public game_object_t {
	int32_t energy;
	int32_t energy_capacity;
	int32_t ticks_to_regeneration;

	template <class Memory>
	void serialize(Memory& memory) {
		game_object_t::serialize(memory);
		memory & energy & energy_capacity & ticks_to_regeneration;
	}
};

struct tombstone_t : public game_object_t {
	creep_t creep;
	int32_t death_time;
	resource_store_t store;
	int32_t ticks_to_decay;

	template <class Memory>
	void serialize(Memory& memory) {
		game_object_t::serialize(memory);
		memory & creep & death_time & store & ticks_to_decay;
	}
};

class room_t {
	friend class game_state_t;
	private:
		template <class Type>
		using container_t = typename internal::memory_range_t<Type>::container_t;

		internal::memory_range_t<creep_t> creeps_memory;
		internal::memory_range_t<dropped_resource_t> dropped_resources_memory;
		internal::memory_range_t<source_t> sources_memory;
		internal::memory_range_t<structure_union_t> structures_memory;
		internal::memory_range_t<tombstone_t> tombstones_memory;
		mineral_t mineral_holder;

	public:
		room_location_t location;
		int32_t energy_available;
		int32_t energy_capacity_available;
		controller_t* controller = nullptr;
		mineral_t* mineral = nullptr;
		// storage_t* storage = nullptr;
		// terminal_t* terminal = nullptr;

		pointer_container_t<construction_site_t> construction_sites;
		container_t<creep_t> creeps;
		container_t<dropped_resource_t> dropped_resources;
		pointer_container_t<flag_t> flags;
		container_t<source_t> sources;
		container_t<structure_union_t> structures;
		container_t<tombstone_t> tombstones;

	private:
		template <class Function>
		void invoke_containers_helper(Function& /* function */) {}

		template <class Function, class Container, class Range, class... Rest>
		void invoke_containers_helper(Function& function, Container& container, Range& range, Rest&... rest) {
			function(container, range);
			invoke_containers_helper(function, rest...);
		}

		template <class Function>
		void invoke_containers(Function function) {
			invoke_containers_helper(
				function,
				creeps, creeps_memory,
				dropped_resources, dropped_resources_memory,
				sources, sources_memory,
				structures, structures_memory,
				tombstones, tombstones_memory
			);
		}

		static void init();
	public:
		static void ensure_capacity(room_t* room);
	private:
		void reset();
		void shrink();
		void update_pointers();

	public:
		template <class Memory>
		void serialize(Memory& memory) {
			memory & location;
			memory & mineral_holder & reinterpret_cast<int&>(mineral);
			memory & energy_available & energy_capacity_available;
			memory & creeps & dropped_resources & sources & structures & tombstones;
			if constexpr (Memory::is_reader) {
				update_pointers();
			}
		}

		int create_construction_site(position_t pos, structure_t::type_t structure_type, const std::string& name = "") const;
		int create_construction_site(local_position_t pos, structure_t::type_t structure_type, const std::string& name = "") const;

		const terrain_t& terrain() const {
			return location.terrain();
		}
};

} // namespace screeps
