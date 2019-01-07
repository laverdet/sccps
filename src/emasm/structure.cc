#include "./javascript.h"
#include <screeps/structure.h>

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
spawn_t::body_t:: body_t(const std::vector<bodypart_t>& parts) : internal::js_handle_t([&]() {
	return EM_ASM_INT({
		var body = [];
		for (var ii = 0; ii < $1; ++ii) {
			body.push(Module.screeps.object.readCreepBodyPart(Module.readInt32($0 + ii * 4)));
		}
		return Module.screeps.util.handleCtor(body);
	}, parts.data(), parts.size());
}()) {
}

int spawn_t::spawn_creep(const body_t& body, const std::string& name) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).spawnCreep(
			Module.screeps.util.handleGet($1),
			Module.screeps.string.readOneByteStringData(Module, $2, $3)
		);
	}, &this->id, internal::js_handle_t::get_ref(body), name.c_str(), name.length());
}

} // namespace screeps
