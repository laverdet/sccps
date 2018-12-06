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
			'extension': {
				'energy': $5,
				'energyCapacity': $6,
			},
			'spawn': {
				'energy': $7,
				'energyCapacity': $8,
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
		// extension
		offsetof(extension_t, energy),
		offsetof(extension_t, energy_capacity),
		// spawn
		offsetof(spawn_t, energy),
		offsetof(spawn_t, energy_capacity)
	);
}

std::ostream& operator<<(std::ostream& os, const structure_t& that) {
	os <<"structure_t[";
	switch (that.type) {
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
	os <<", [" <<that.pos.room_location() <<" " <<that.pos.xx % 50 <<", " <<that.pos.yy % 50 <<"], " <<that.id <<"]";
	return os;
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
