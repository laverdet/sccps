#include "game.h"
#include <emscripten.h>

namespace screeps {

static mineral_t mineral_hole;

/**
 * game_t implementation
 */
void game_state_t::load() {
	resource_store_t::preloop();

	rooms.clear();
	creeps_by_id.clear();
	dropped_resources_by_id.clear();
	sources_by_id.clear();
	structures_by_id.clear();
	tombstones_by_id.clear();

	creeps.clear();
	sources.clear();
	structures.clear();

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
void game_state_t::init() {
	creep_t::init();
	resource_store_t::init();
	room_t::init();
	structure_t::init();
}

EMSCRIPTEN_KEEPALIVE
void game_state_t::flush_game(
	game_state_t* game,
	uint32_t creeps_count,
	uint32_t dropped_resources_count,
	uint32_t sources_count,
	uint32_t structures_count,
	uint32_t tombstones_count
) {
	game->creeps_by_id.reserve(creeps_count);
	game->dropped_resources_by_id.reserve(dropped_resources_count);
	game->sources_by_id.reserve(sources_count);
	game->structures_by_id.reserve(structures_count);
	game->tombstones_by_id.reserve(tombstones_count);
	for (auto& room : game->rooms) {
		for (auto& creep : room.second.creeps) {
			game->creeps_by_id.emplace(creep.id, &creep);
		}
	}
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

EMSCRIPTEN_KEEPALIVE
void game_state_t::reserve_rooms(game_state_t* game, uint32_t rooms_count) {
	game->rooms.reserve(rooms_count);
}

// Called in the case of an uncaught exception. This does the `what()` virtual function call and
// also returns RTTI.
EMSCRIPTEN_KEEPALIVE
void* exception_what(void* ptr) {
	struct hole_t {
		const char* name;
		const char* what;
	};
	static hole_t hole;
	auto& err = *reinterpret_cast<const std::exception*>(ptr);
	hole.name = typeid(err).name();
	hole.what = err.what();
	return reinterpret_cast<void*>(&hole);
}

} // namespace screeps
