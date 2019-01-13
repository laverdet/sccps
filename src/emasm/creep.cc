#include "./javascript.h"
#include <screeps/creep.h>
#include <cstring>

namespace screeps {

int creep_t::static_method::build(const sid_t& creep, const sid_t& target) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).build(Module.screeps.util.getObjectById(Module, $1));
	}, &creep, &target);
}

int creep_t::static_method::cancel_order(const sid_t& creep, const char* method) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).cancelOrder(Module.screeps.string.readOneByteStringData(Module, $1, $2));
	}, &creep, method, strlen(method));
}

int creep_t::static_method::drop(const sid_t& creep, resource_t resource, int amount) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).drop(
			Module.screeps.object.readResourceType($1),
			$2 === -1 ? undefined : $2
		);
	}, &creep, resource, amount);
}

int creep_t::static_method::harvest(const sid_t& creep, const sid_t& target) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).harvest(Module.screeps.util.getObjectById(Module, $1));
	}, &creep, &target);
}

int creep_t::static_method::move(const sid_t& creep, direction_t direction) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).move($1);
	}, &creep, direction);
}

int creep_t::static_method::pickup(const sid_t& creep, const sid_t& target) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).pickup(Module.screeps.util.getObjectById(Module, $1));
	}, &creep, &target);
}

int creep_t::static_method::repair(const sid_t& creep, const sid_t& target) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).repair(Module.screeps.util.getObjectById(Module, $1));
	}, &creep, &target);
}

int creep_t::static_method::suicide(const sid_t& creep) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).suicide();
	}, &creep);
}

int creep_t::static_method::transfer(const sid_t& creep, const sid_t& target, resource_t resource, int amount) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).transfer(
			Module.screeps.util.getObjectById(Module, $1),
			Module.screeps.object.readResourceType($2),
			$3 === -1 ? undefined : $3
		);
	}, &creep, &target, resource, amount);
}

int creep_t::static_method::upgrade_controller(const sid_t& creep, const sid_t& target) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).upgradeController(Module.screeps.util.getObjectById(Module, $1));
	}, &creep, &target);
}

int creep_t::static_method::withdraw(const sid_t& creep, const sid_t& target, resource_t resource, int amount) {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).withdraw(
			Module.screeps.util.getObjectById(Module, $1),
			Module.screeps.object.readResourceType($2),
			$3 === -1 ? undefined : $3
		);
	}, &creep, &target, resource, amount);
}

void creep_t::init() {
	EM_ASM({
		Module.screeps.object.initCreepLayout({
			'sizeof': $0,
			'body': $1,
			'carry': $2,
			'carryCapacity': $3,
			'fatigue': $4,
			'hits': $5,
			'hitsMax': $6,
			'my': $7,
			'name': $8,
			'spawning': $9,
			'ticksToLive': $10,
			'bodyPartSizeof': $11,
			'bodyPartBoost': $12,
			'bodyPartType': $13
		});
	},
		sizeof(creep_t),
		offsetof(creep_t, body),
		offsetof(creep_t, carry),
		offsetof(creep_t, carry_capacity),
		offsetof(creep_t, fatigue),
		offsetof(creep_t, hits),
		offsetof(creep_t, hits_max),
		offsetof(creep_t, my),
		offsetof(creep_t, name),
		offsetof(creep_t, spawning),
		offsetof(creep_t, ticks_to_live),
		sizeof(creep_bodypart_t),
		offsetof(creep_bodypart_t, boost),
		offsetof(creep_bodypart_t, type)
	);
}

} // namespace screeps
