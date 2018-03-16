#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#include "game.h"

static mineral_t mineral_hole;

/**
 * game_t implementation
 */
void game_state_t::clear() {
	creeps.clear();
	rooms.clear();
	sources.clear();
	structures.clear();
}

void game_state_t::load() {
	resource_store_t::preloop();
	clear();
	EM_ASM({
		Module.screeps.game.write(Module, $0, $1, $2, $3, $4, $5, $6, $7);
	},
		this,
		&creeps,
		&dropped_resources,
		&flags,
		&sources,
		&structures,
		&tombstones,
		&mineral_hole
	);
}

EMSCRIPTEN_KEEPALIVE
void game_state_t::flush_room(
	game_state_t* game,
	uint32_t rx, uint32_t ry,
	uint32_t energy_available, uint32_t energy_capacity_available,
	void* creeps_begin, void* creeps_end,
	void* dropped_resources_begin, void* dropped_resources_end,
	void* sources_begin, void* sources_end,
	void* structure_begin, void* structure_end,
	void* tombstones_begin, void* tombstones_end
) {
	room_location_t room(rx, ry);
	game->rooms.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(room),
		std::forward_as_tuple(
			room,
			energy_available, energy_capacity_available,
			mineral_hole,
			reinterpret_cast<creep_t*>(creeps_begin), reinterpret_cast<creep_t*>(creeps_end),
			reinterpret_cast<dropped_resource_t*>(dropped_resources_begin), reinterpret_cast<dropped_resource_t*>(dropped_resources_end),
			reinterpret_cast<source_t*>(sources_begin), reinterpret_cast<source_t*>(sources_end),
			reinterpret_cast<structure_union_t*>(structure_begin), reinterpret_cast<structure_union_t*>(structure_end),
			reinterpret_cast<tombstone_t*>(tombstones_begin), reinterpret_cast<tombstone_t*>(tombstones_end)
		)
	);
}

void init() {
	creep_t::init();
	resource_store_t::init();
	room_t::init();
	structure_t::init();
}

EMSCRIPTEN_BINDINGS(screeps) {
	emscripten::function("init", &init);
}
