#include "./javascript.h"
#include <screeps/creep.h>
#include <cstring>

namespace screeps {

void creep_t::init() {
	EM_ASM({
		Module.screeps.object.initCreepLayout({
			'sizeof': $0,
			'body': $1,
			'carry': $2,
			'fatigue': $3,
			'hits': $4,
			'hitsMax': $5,
			'my': $6,
			'name': $7,
			'spawnId': $8,
			'isSpawning': $9,
			'ticksToLive': $10,
			'bodyPartSizeof': $11,
			'bodyPartBoost': $12,
			'bodyPartType': $13
		});
	},
		sizeof(creep_t),
		offsetof(creep_t, body),
		offsetof(creep_t, carry),
		offsetof(creep_t, fatigue),
		offsetof(creep_t, hits),
		offsetof(creep_t, hits_max),
		offsetof(creep_t, my),
		offsetof(creep_t, name),
		offsetof(creep_t, _spawn_id),
		offsetof(creep_t, _is_spawning),
		offsetof(creep_t, ticks_to_live),
		sizeof(creep_bodypart_t),
		offsetof(creep_bodypart_t, boost),
		offsetof(creep_bodypart_t, type)
	);
}

const std::vector<creep_active_bodypart_t> creep_t::get_active_bodyparts() const {
	std::vector<creep_active_bodypart_t> parts;
	parts.reserve(hits / 100);
	int active_hits = hits;
	for (auto& ii : body) {
		if (active_hits > 100) {
			parts.emplace_back(ii, 100);
			active_hits -= 100;
		} else {
			parts.emplace_back(ii, active_hits);
			break;
		}
	}
	return parts;
}

#ifdef JAVASCRIPT
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
#else
int creep_t::static_method::build(const sid_t& creep, const sid_t& target) {
	std::cerr <<creep <<".build(" <<target <<")\n";
	return 0;
}

int creep_t::static_method::cancel_order(const sid_t& creep, const char* method) {
	std::cerr <<creep <<".cancel_order(" <<method <<")\n";
	return 0;
}

int creep_t::static_method::drop(const sid_t& creep, resource_t resource, int amount) {
	std::cerr <<creep <<".drop(" <<resource <<", " <<amount <<")\n";
	return 0;
}

int creep_t::static_method::harvest(const sid_t& creep, const sid_t& target) {
	std::cerr <<creep <<".harvest(" <<target <<")\n";
	return 0;
}

int creep_t::static_method::move(const sid_t& creep, direction_t direction) {
	std::cerr <<creep <<".move(" <<direction <<")\n";
	return 0;
}

int creep_t::static_method::pickup(const sid_t& creep, const sid_t& target) {
	std::cerr <<creep <<".pickup(" <<target <<")\n";
	return 0;
}

int creep_t::static_method::repair(const sid_t& creep, const sid_t& target) {
	std::cerr <<creep <<".repair(" <<target <<")\n";
	return 0;
}

int creep_t::static_method::suicide(const sid_t& creep) {
	std::cerr <<creep <<".suicide()\n";
	return 0;
}

int creep_t::static_method::transfer(const sid_t& creep, const sid_t& target, resource_t resource, int amount) {
	std::cerr <<creep <<".transfer(" <<target <<", " <<resource <<", " <<amount <<")\n";
	return 0;
}

int creep_t::static_method::upgrade_controller(const sid_t& creep, const sid_t& target) {
	std::cerr <<creep <<".upgrade_controller(" <<target <<")\n";
	return 0;
}

int creep_t::static_method::withdraw(const sid_t& creep, const sid_t& target, resource_t resource, int amount) {
	std::cerr <<creep <<".transfer(" <<target <<", " <<resource <<", " <<amount <<")\n";
	return 0;
}
#endif

std::ostream& operator<<(std::ostream& os, const creep_t& that) {
	if (that.my) {
		return os <<"creep_t[" <<that.name <<"]";
	} else {
		return os <<"creep_t[!" <<that.id <<"]";
	}
}

std::ostream& operator<<(std::ostream& os, bodypart_t type) {
	os <<"bodypart_t::";
	switch (type) {
		case bodypart_t::none: return os <<"none";
		case bodypart_t::attack: return os <<"attack";
		case bodypart_t::carry: return os <<"carry";
		case bodypart_t::claim: return os <<"claim";
		case bodypart_t::heal: return os <<"heal";
		case bodypart_t::move: return os <<"move";
		case bodypart_t::ranged_attack: return os <<"ranged_attack";
		case bodypart_t::tough: return os <<"tough";
		case bodypart_t::work: return os <<"work";
	}
}


} // namespace screeps
