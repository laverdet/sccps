#include <screeps/room.h>
#include <emscripten.h>

namespace screeps {

void room_t::init() {
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

} // namespace screeps
