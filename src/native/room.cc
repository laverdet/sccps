#include <screeps/room.h>

namespace screeps {

room_t::room_t(
	room_location_t location,
	int energy_available, int energy_capacity_available,
	const mineral_t* mineral,
	construction_site_t* construction_sites_begin, construction_site_t* construction_sites_end,
	creep_t* creeps_begin, creep_t* creeps_end,
	dropped_resource_t* dropped_resources_begin, dropped_resource_t* dropped_resources_end,
	source_t* sources_begin, source_t* sources_end,
	structure_union_t* structures_begin, structure_union_t* structures_end,
	tombstone_t* tombstones_begin, tombstone_t* tombstones_end
) :
	location(location),
	energy_available(energy_available), energy_capacity_available(energy_capacity_available),
	mineral(mineral == nullptr ? std::nullopt : std::optional<mineral_t>(*mineral)),
	construction_sites(construction_sites_begin, construction_sites_end),
	creeps(creeps_begin, creeps_end),
	dropped_resources(dropped_resources_begin, dropped_resources_end),
	sources(sources_begin, sources_end),
	structures(structures_begin, structures_end),
	tombstones(tombstones_begin, tombstones_end
) {
	for (auto& ii : structures) {
		if (ii.type == structure_t::controller) {
			controller = &ii.controller;
			break;
		}
	}
}

} // namespace screeps
