#include <screeps/game.h>

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
}

} // namespace screeps
