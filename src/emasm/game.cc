#include <screeps/game.h>
#include <emscripten.h>

namespace screeps {

/**
 * game_t implementation
 */
void game_state_t::load() {
	resource_store_t::preloop();

	rooms.clear();
	construction_sites_by_id.clear();
	creeps_by_id.clear();
	creeps_by_name.clear();
	dropped_resources_by_id.clear();
	sources_by_id.clear();
	structures_by_id.clear();
	tombstones_by_id.clear();

	construction_sites.clear();
	creeps.clear();
	dropped_resources.clear();
	sources.clear();
	structures.clear();

	EM_ASM({
		Module.screeps.object.writeGame(Module, $0);
	}, this);
}

EMSCRIPTEN_KEEPALIVE
void game_state_t::init() {
	static mineral_t mineral_hole;
	EM_ASM({
		Module.screeps.object.initGameLayout({
			'constructionSites': $0,
			'creeps': $1,
			'droppedResources': $2,
			'flags': $3,
			'sources': $4,
			'structures': $5,
			'tombstones': $6,

			'gcl': $7,
			'mineralHole': $8,
			'time': $9,
		});
	},
		offsetof(game_state_t, construction_sites),
		offsetof(game_state_t, creeps),
		offsetof(game_state_t, dropped_resources),
		offsetof(game_state_t, flags),
		offsetof(game_state_t, sources),
		offsetof(game_state_t, structures),
		offsetof(game_state_t, tombstones),

		offsetof(game_state_t, gcl),
		&mineral_hole,
		offsetof(game_state_t, time)
	);
	EM_ASM({
		Module.screeps.object.initDroppedResourceLayout({
			'droppedResource': {
				'sizeof': $0,
				'amount': $1,
				'resourceType': $2,
			},
		});
	},
		sizeof(dropped_resource_t),
		offsetof(dropped_resource_t, amount),
		offsetof(dropped_resource_t, type)
	);
	creep_t::init();
	resource_store_t::init();
	room_t::init();
	structure_t::init();
}

EMSCRIPTEN_KEEPALIVE
void game_state_t::flush_game(game_state_t* game) {
	game->construction_sites_by_id.reserve(game->construction_sites.size());
	game->creeps_by_id.reserve(game->creeps.size());
	game->dropped_resources_by_id.reserve(game->dropped_resources.size());
	game->sources_by_id.reserve(game->sources.size());
	game->structures_by_id.reserve(game->structures.size());
	game->tombstones_by_id.reserve(game->tombstones.size());
	for (auto& room : game->rooms) {
		for (auto& construction_site : room.second.construction_sites) {
			game->construction_sites_by_id.emplace(construction_site.id, &construction_site);
		}
		for (auto& creep : room.second.creeps) {
			game->creeps_by_id.emplace(creep.id, &creep);
		}
		for (auto& dropped_resource : room.second.dropped_resources) {
			game->dropped_resources_by_id.emplace(dropped_resource.id, &dropped_resource);
		}
		for (auto& source : room.second.sources) {
			game->sources_by_id.emplace(source.id, &source);
		}
		for (auto& structure : room.second.structures) {
			game->structures_by_id.emplace(structure.id, &structure);
		}
	}
}

EMSCRIPTEN_KEEPALIVE
void game_state_t::flush_room(
	game_state_t* game,
	int rx, int ry,
	int energy_available, int energy_capacity_available,
	void* mineral_ptr,
	void* construction_sites_begin, void* construciton_sites_end,
	void* creeps_begin, void* creeps_end,
	void* dropped_resources_begin, void* dropped_resources_end,
	void* sources_begin, void* sources_end,
	void* structure_begin, void* structure_end,
	void* tombstones_begin, void* tombstones_end
) {
	room_location_t room(rx, ry);
	auto& room_ref = *game->rooms.emplace(
		std::piecewise_construct,
		std::forward_as_tuple(room),
		std::forward_as_tuple(
			room,
			energy_available, energy_capacity_available,
			reinterpret_cast<mineral_t*>(mineral_ptr),
			reinterpret_cast<construction_site_t*>(construction_sites_begin), reinterpret_cast<construction_site_t*>(construciton_sites_end),
			reinterpret_cast<creep_t*>(creeps_begin), reinterpret_cast<creep_t*>(creeps_end),
			reinterpret_cast<dropped_resource_t*>(dropped_resources_begin), reinterpret_cast<dropped_resource_t*>(dropped_resources_end),
			reinterpret_cast<source_t*>(sources_begin), reinterpret_cast<source_t*>(sources_end),
			reinterpret_cast<structure_union_t*>(structure_begin), reinterpret_cast<structure_union_t*>(structure_end),
			reinterpret_cast<tombstone_t*>(tombstones_begin), reinterpret_cast<tombstone_t*>(tombstones_end)
		)
	).first;
	for (auto& creep : room_ref.second.creeps) {
		game->creeps_by_name.emplace(creep.name, &creep);
	}
}

EMSCRIPTEN_KEEPALIVE
void game_state_t::reserve_rooms(game_state_t* game, int rooms_count) {
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

// Keep loop function alive
EMSCRIPTEN_KEEPALIVE void loop();
