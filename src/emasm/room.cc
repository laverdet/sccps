#include <screeps/room.h>
#include <emscripten.h>

namespace screeps {

void room_t::init() {
	// room_t
	EM_ASM({
		Module.screeps.object.initRoomLayout({
			'location': $0,
			'creeps': $1,
			'droppedResources': $2,
			'sources': $3,
			'structures': $4,
			'tombstones': $5,
			'mineral': $6,
			'mineralHolder': $7,
		});
	},
		offsetof(room_t, location),
		offsetof(room_t, creeps_memory),
		offsetof(room_t, dropped_resources_memory),
		offsetof(room_t, sources_memory),
		offsetof(room_t, structures_memory),
		offsetof(room_t, tombstones_memory),
		offsetof(room_t, mineral),
		offsetof(room_t, mineral_holder)
	);

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

EMSCRIPTEN_KEEPALIVE
void room_t::ensure_capacity(room_t* room) {
	room->invoke_containers([&](auto& container, auto& memory) {
		memory.ensure_capacity(container);
	});
}

void room_t::reset() {
	invoke_containers([&](auto& container, auto& memory) {
		memory.reset(container);
	});
}

void room_t::shrink() {
	invoke_containers([&](auto& container, auto& memory) {
		memory.shrink(container);
	});
}

} // namespace screeps
