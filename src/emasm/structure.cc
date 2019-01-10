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
				'spawning': $14,
				'spawningDirections': $15,
				'spawningNeedTime': $16,
				'spawningRemainingTime': $17,
				'spawningName': $18,
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
		offsetof(spawn_t, energy_capacity),
		offsetof(spawn_t, _is_spawning),
		offsetof(spawn_t, spawning) + offsetof(spawn_t::spawning_t, directions),
		offsetof(spawn_t, spawning) + offsetof(spawn_t::spawning_t, need_time),
		offsetof(spawn_t, spawning) + offsetof(spawn_t::spawning_t, remaining_time),
		offsetof(spawn_t, spawning) + offsetof(spawn_t::spawning_t, name)
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

int spawn_t::spawn_creep(const body_t& body, const std::string& name, const options_t& options) const {
	return EM_ASM_INT({
		let options;
		if ($4 !== 0) {
			options = { directions: [] };
			do {
				let direction = $4 & 0x0f;
				options.directions.push(direction);
				$4 >>>= 4;
			} while ($4 !== 0);
		}
		return Module.screeps.util.getObjectById(Module, $0).spawnCreep(
			Module.screeps.util.handleGet($1),
			Module.screeps.string.readOneByteStringData(Module, $2, $3),
			options
		);
	}, &this->id, internal::js_handle_t::get_ref(body), name.c_str(), name.length(), options.directions.bits);
}

void spawn_t::spawning_t::cancel() const {
	EM_ASM({
		Module.screeps.util.getObjectById(Module, $0).spawning.cancel();
	}, &this->spawn().id);
}

void spawn_t::spawning_t::set_directions(directions_t directions) const {
	EM_ASM({
		let directions;
		if ($1 !== 0) {
			directions = [];
			do {
				let direction = $1 & 0x0f;
				directions.push(direction);
				$1 >>>= 4;
			} while ($1 !== 0);
		}
		Module.screeps.util.getObjectById(Module, $0).spawning.setDirections(directions);
	}, &this->spawn().id, directions.bits);
}

} // namespace screeps
