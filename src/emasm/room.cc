#include <screeps/room.h>
#include <emscripten.h>

namespace screeps {

void room_t::init() {
	// construction_site_t
	EM_ASM({
		Module.screeps.object.initConstructionSiteLayout({
			'sizeof': $0,
			'my': $1,
			'progress': $2,
			'progressTotal': $3,
			'structureType': $4,
		});
	},
		sizeof(construction_site_t),
		offsetof(construction_site_t, my),
		offsetof(construction_site_t, progress),
		offsetof(construction_site_t, progress_total),
		offsetof(construction_site_t, type)
	);

	// dropped_resource_t
	EM_ASM({
		Module.screeps.object.initDroppedResourceLayout({
			'sizeof': $0,
			'amount': $1,
			'resourceType': $2,
		});
	},
		sizeof(dropped_resource_t),
		offsetof(dropped_resource_t, amount),
		offsetof(dropped_resource_t, type)
	);

	// mineral_t
	EM_ASM({
		Module.screeps.object.initMineralLayout({
			'sizeof': $0,
			'amount': $2,
			'density': $3,
			'mineralType': $1,
			'ticksToRegeneration': $4,
		});
	},
		sizeof(mineral_t),
		offsetof(mineral_t, amount),
		offsetof(mineral_t, density),
		offsetof(mineral_t, type),
		offsetof(mineral_t, ticks_to_regeneration)
	);

	// source_t
	EM_ASM({
		Module.screeps.object.initSourceLayout({
			'sizeof': $0,
			'energy': $1,
			'energyCapacity': $2,
			'ticksToRegeneration': $3,
		});
	},
		sizeof(source_t),
		offsetof(source_t, energy),
		offsetof(source_t, energy_capacity),
		offsetof(source_t, ticks_to_regeneration)
	);
}

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
