#include "./room.h"
#include <emscripten.h>

void room_t::init() {
	EM_ASM({
		Module.screeps.room.init({
			'flag': {
				'sizeof': $0,
				'color': $1,
				'secondaryColor': $2,
			},
			'mineral': {
				'size': $3,
				'type': $4,
				'amount': $5,
				'density': $6,
				'ticksToRegeneration': $7,
			},
			'source': {
				'sizeof': $8,
				'energy': $9,
				'energyCapacity': $10,
				'ticksToRegeneration': $11,
			},
			'resource': {
				'sizeof': $12,
				'resourceType': $13,
				'amount': $14,
			},
			'tombstone': {
				'sizeof': $15,
				'creep': $16,
				'deathTime': $17,
				'store': $18,
				'ticksToDecay': $19,
			},
		});
	},
		sizeof(flag_t),
		offsetof(flag_t, color),
		offsetof(flag_t, secondary_color),
		sizeof(mineral_t),
		offsetof(mineral_t, type),
		offsetof(mineral_t, amount),
		offsetof(mineral_t, density),
		offsetof(mineral_t, ticks_to_regeneration),
		sizeof(source_t),
		offsetof(source_t, energy),
		offsetof(source_t, energy_capacity),
		offsetof(source_t, ticks_to_regeneration),
		sizeof(dropped_resource_t),
		offsetof(dropped_resource_t, type),
		offsetof(dropped_resource_t, amount),
		sizeof(tombstone_t),
		offsetof(tombstone_t, creep),
		offsetof(tombstone_t, death_time),
		offsetof(tombstone_t, store),
		offsetof(tombstone_t, ticks_to_decay)
	);
}
