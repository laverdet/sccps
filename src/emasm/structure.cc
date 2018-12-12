#include <screeps/structure.h>
#include <emscripten.h>

namespace screeps {

void structure_t::init() {
	EM_ASM({
		Module.screeps.object.initStructureLayout({
			'sizeof': $0,
			'structure': {
				'structureType': $1,
			},
			'destroyable': {
				'hits': $2,
				'hitsMax': $3,
			},
			'owned': {
				'my': $4,
			},
			'controller': {
				'level': $5,
				'progress': $6,
				'progressTotal': $7,
				'ticksToDowngrade': $8,
				'upgradeBlocked': $9,
			},
			'extension': {
				'energy': $10,
				'energyCapacity': $11,
			},
			'spawn': {
				'energy': $12,
				'energyCapacity': $13,
			},
		});
	},
		// structure
		sizeof(structure_union_t),
		offsetof(structure_t, type),
		// destroyable structure
		offsetof(destroyable_structure_t, hits),
		offsetof(destroyable_structure_t, hits_max),
		// owned structure
		offsetof(owned_structure_t, my),
		// controller
		offsetof(controller_t, level),
		offsetof(controller_t, progress),
		offsetof(controller_t, progress_total),
		offsetof(controller_t, ticks_to_downgrade),
		offsetof(controller_t, upgrade_blocked),
		// extension
		offsetof(extension_t, energy),
		offsetof(extension_t, energy_capacity),
		// spawn
		offsetof(spawn_t, energy),
		offsetof(spawn_t, energy_capacity)
	);
}

/**
 * spawn_t
 */
int spawn_t::spawn_creep(const creep_body_t& body, const std::string& name) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).spawnCreep(
			Module.screeps.object.readCreepBodyPartArray(Module, $1),
			Module.screeps.string.readOneByteStringData(Module, $2, $3)
		);
	}, &this->id, &body, name.c_str(), name.length());
}

} // namespace screeps
