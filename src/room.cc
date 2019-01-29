#include "./javascript.h"
#include <screeps/room.h>

namespace screeps {

void room_t::init() {
	// room_t
	EM_ASM({
		Module.screeps.object.initRoomLayout({
			'location': $0,
			'energyAvailable': $1,
			'energyCapacityAvailable': $2,
			'creeps': $3,
			'droppedResources': $4,
			'sources': $5,
			'structures': $6,
			'tombstones': $7,
			'mineral': $8,
			'mineralHolder': $9,
		});
	},
		offsetof(room_t, location),
		offsetof(room_t, energy_available),
		offsetof(room_t, energy_capacity_available),
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

int room_t::create_construction_site(position_t pos, structure_t::type_t structure_type, const std::string& name) const {
	if (pos.room_location() != location) {
		// JS @screeps/engine doesn't actually check this..
		return k_err_invalid_args;
	}
	return create_construction_site(pos.to_local(), structure_type, name);
}

int room_t::create_construction_site(local_position_t pos, structure_t::type_t structure_type, const std::string& name) const {
#ifdef JAVASCRIPT
	return EM_ASM_INT({
		return Game.rooms[
			Module.screeps.position.generateRoomName($0)
		].createConstructionSite(
			$1, $2,
			Module.screeps.object.readStructureType($3),
			Module.screeps.string.readOneByteStringData(Module, $4, $5)
		);
	},
		location.id,
		pos.xx, pos.yy,
		structure_type,
		name.data(), name.size()
	);
#else
	std::cerr <<location <<".create_construction_site(" <<pos <<", " <<structure_type <<", \"" <<name <<"\")\n";
	return 0;
#endif
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

void room_t::update_pointers() {
	construction_sites = {nullptr, nullptr};
	flags = {nullptr, nullptr};
	if (mineral != nullptr) {
		mineral = &mineral_holder;
	}
	controller = nullptr;
/*
	storage = nullptr;
	terminal = nullptr;
*/
	for (auto& structure : structures) {
		if (auto controller = structure.get<controller_t>(); controller != nullptr) {
			this->controller = controller;
		}
/*
			case structure_t::storage:
				storage = &structure;
				break;
			case structure_t::terminal:
				terminal = &structure;
				break;
*/
	}
}

} // namespace screeps
