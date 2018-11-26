#pragma once
#include <cstdint>
#include "./creep.h"
#include "./object.h"

namespace screeps {

/**
 * unowned structures:
 * container, road, wall, portal
 */

struct structure_t : game_object_t {
	enum type_t {
		container,
		controller,
		extension,
		extractor,
		keeper_lair,
		lab,
		link,
		nuker,
		observer,
		portal,
		power_bank,
		power_spawn,
		rampart,
		road,
		spawn,
		storage,
		terminal,
		tower,
		wall,
	};

	const type_t type; // must be first!

	static void init();
	friend std::ostream& operator<<(std::ostream& os, const structure_t& that);
};

struct destroyable_structure_t : structure_t {
	uint32_t hits;
	uint32_t hits_max;
};

struct owned_structure_t : destroyable_structure_t {
	bool my;
};

struct controller_t : structure_t {
	uint32_t level;
	uint32_t progress;
	uint32_t progress_total;
	uint32_t ticks_to_downgrade;
	uint32_t upgrade_blocked;
};

struct extension_t : owned_structure_t {
	uint32_t energy;
	uint32_t energy_capacity;
};

struct spawn_t : owned_structure_t {
	uint32_t energy;
	uint32_t energy_capacity;

	int spawn_creep(const creep_body_t& body, const std::string& name) const;
};

// Used to reserve enough space in the array memory for any structure
union structure_union_t {
	struct {
		position_t pos;
		id_t id;
		structure_t::type_t type;
	};

	bool is_owned() const {
		switch (type) {
			case structure_t::extension:
			case structure_t::extractor:
			case structure_t::lab:
			case structure_t::link:
			case structure_t::nuker:
			case structure_t::observer:
			case structure_t::power_spawn:
			case structure_t::rampart:
			case structure_t::spawn:
			case structure_t::storage:
			case structure_t::terminal:
			case structure_t::tower:
			case structure_t::wall:
				return true;
			default:
				return false;
		}
	}

	operator game_object_t&() {
		return *reinterpret_cast<game_object_t*>(this);
	}

	operator const game_object_t&() const {
		return *reinterpret_cast<const game_object_t*>(this);
	}

	destroyable_structure_t destroyable;
	owned_structure_t owned;

	controller_t controller;
	extension_t extension;
	spawn_t spawn;
};

} // namespace screeps
