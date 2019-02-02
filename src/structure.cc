#include "./javascript.h"
#include <screeps/structure.h>

namespace screeps {

void structure_t::init() {
	// Base class
	EM_ASM({
		Module.screeps.object.initStructureLayout({
			'sizeof': $0,
			'structureType': $1,
			'hits': $2,
			'hitsMax': $3,
			'owner': $4,
			'my': $5,
		});
	},
		sizeof(structure_union_t),
		offsetof(structure_t, type),
		offsetof(structure_t, hits),
		offsetof(structure_t, hits_max),
		offsetof(structure_t, owner),
		offsetof(structure_t, my)
	);
	// Container
	EM_ASM({
		Module.screeps.object.initStructureContainerLayout({
			'store': $0,
			'ticksToDecay': $1,
		});
	},
		offsetof(container_t, store),
		offsetof(container_t, ticks_to_decay)
	);
	// Controller
	EM_ASM({
		Module.screeps.object.initStructureControllerLayout({
			'level': $0,
			'progress': $1,
			'progressTotal': $2,
			'ticksToDowngrade': $3,
			'upgradeBlocked': $4,
		});
	},
		offsetof(controller_t, level),
		offsetof(controller_t, progress),
		offsetof(controller_t, progress_total),
		offsetof(controller_t, ticks_to_downgrade),
		offsetof(controller_t, upgrade_blocked)
	);
	// Extension
	EM_ASM({
		Module.screeps.object.initStructureExtensionLayout({
			'energy': $0,
			'energyCapacity': $1,
		});
	},
		offsetof(extension_t, energy),
		offsetof(extension_t, energy_capacity)
	);
	// Road
	EM_ASM({
		Module.screeps.object.initStructureRoadLayout({
			'ticksToDecay': $0
		});
	},
		offsetof(road_t, ticks_to_decay)
	);
	// Spawn
	EM_ASM({
		Module.screeps.object.initStructureSpawnLayout({
			'energy': $0,
			'energyCapacity': $1,
			'spawning': $2,
			'spawningDirections': $3,
			'spawningNeedTime': $4,
			'spawningRemainingTime': $5,
			'spawningId': $6,
		});
	},
		offsetof(spawn_t, energy),
		offsetof(spawn_t, energy_capacity),
		offsetof(spawn_t, _is_spawning),
		offsetof(spawn_t, spawning) + offsetof(spawn_t::spawning_t, directions),
		offsetof(spawn_t, spawning) + offsetof(spawn_t::spawning_t, need_time),
		offsetof(spawn_t, spawning) + offsetof(spawn_t::spawning_t, remaining_time),
		offsetof(spawn_t, spawning) + offsetof(spawn_t::spawning_t, id)
	);
}

/**
 * spawn_t
 */
#ifdef JAVASCRIPT
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
		var options;
		if ($4 !== 0) {
			options = { directions: [] };
			do {
				var direction = $4 & 0x0f;
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
		var directions;
		if ($1 !== 0) {
			directions = [];
			do {
				var direction = $1 & 0x0f;
				directions.push(direction);
				$1 >>>= 4;
			} while ($1 !== 0);
		}
		Module.screeps.util.getObjectById(Module, $0).spawning.setDirections(directions);
	}, &this->spawn().id, directions.bits);
}
#else
spawn_t::body_t::body_t(const std::vector<bodypart_t>& /* parts */) : internal::js_handle_t(0) {
}

int spawn_t::spawn_creep(const body_t& /* body */, const std::string& name, const options_t& /* options */) const {
	std::cerr <<*this <<".spawn_creep(..., \"" <<name <<"\")\n";
	return 0;
}

void spawn_t::spawning_t::cancel() const {
	std::cerr <<this->spawn() <<".spawning.cancel()\n";
}

void spawn_t::spawning_t::set_directions(directions_t directions) const {
	std::cerr <<this->spawn() <<".spawning.set_directions(...)\n";
}
#endif

std::ostream& operator<<(std::ostream& os, const structure_t& that) {
	os <<"structure_t[";
	switch (that.type) {
		case structure_t::none: os<<"none"; break;
		case structure_t::size: os<<"size"; break;
		case structure_t::container: os <<"container"; break;
		case structure_t::controller: os <<"controller"; break;
		case structure_t::extension: os <<"extension"; break;
		case structure_t::extractor: os <<"extractor"; break;
		case structure_t::keeper_lair: os <<"keeper_lair"; break;
		case structure_t::lab: os <<"lab"; break;
		case structure_t::link: os <<"link"; break;
		case structure_t::nuker: os <<"nuker"; break;
		case structure_t::observer: os <<"observer"; break;
		case structure_t::portal: os <<"portal"; break;
		case structure_t::power_bank: os <<"power_bank"; break;
		case structure_t::power_spawn: os <<"power_spawn"; break;
		case structure_t::rampart: os <<"rampart"; break;
		case structure_t::road: os <<"road"; break;
		case structure_t::spawn: os <<"spawn"; break;
		case structure_t::storage: os <<"storage"; break;
		case structure_t::terminal: os <<"terminal"; break;
		case structure_t::tower: os <<"tower"; break;
		case structure_t::wall: os <<"wall"; break;
	}
	os <<", [" <<that.pos.room <<" " <<that.pos.xx % 50 <<", " <<that.pos.yy % 50 <<"], " <<that.id <<"]";
	return os;
}

} // namespace screeps
