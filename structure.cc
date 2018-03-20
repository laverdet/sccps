#include "./structure.h"
#include <emscripten.h>

namespace screeps {

void structure_t::init() {
	EM_ASM({
		Module.screeps.structure.init({
			'sizeof': $0,
			'structure': {
				'structureType': $1,
				'hits': $2,
				'hitsMax': $3,
			},
			'spawn': {
				'energy': $4,
				'energyCapacity': $5,
			},
		});
	},
		// structure
		sizeof(structure_union_t),
		offsetof(structure_t, type),
		offsetof(structure_with_hits_t, hits),
		offsetof(structure_with_hits_t, hits_max),
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
			Module.screeps.creep.readBodyPartList(Module, $1),
			Module.screeps.string.readOneByteStringData(Module, $2, $3)
		);
	}, &this->id, &body, name.c_str(), name.length());
}

} // namespace screeps
