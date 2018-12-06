#include <screeps/creep.h>
#include <cstring>
#include <emscripten.h>

namespace screeps {

int creep_t::build(const game_object_t& target) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).build(Module.screeps.util.getObjectById(Module, $1));
	}, &this->id, &target.id);
}

int creep_t::cancel_order(const char* method) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).cancelOrder(Module.screeps.string.readOneByteStringData(Module, $1, $2));
	}, &this->id, method, strlen(method));
}

int creep_t::drop(resource_t resource, int amount) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).drop(
			Module.screeps.object.readResourceType($1),
			$2 === -1 ? undefined : $2
		);
	}, &this->id, resource, amount);
}

int creep_t::harvest(const game_object_t& target) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).harvest(Module.screeps.util.getObjectById(Module, $1));
	}, &this->id, &target.id);
}

int creep_t::move(direction_t direction) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).move($1);
	}, &this->id, direction);
}

int creep_t::pickup(const game_object_t& target) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).pickup(Module.screeps.util.getObjectById(Module, $1));
	}, &this->id, &target.id);
}

int creep_t::repair(const game_object_t& target) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).repair(Module.screeps.util.getObjectById(Module, $1));
	}, &this->id, &target.id);
}

int creep_t::suicide() const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).suicide();
	}, &this->id);
}

int creep_t::transfer(const game_object_t& target, resource_t resource, int amount) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).transfer(
			Module.screeps.util.getObjectById(Module, $1),
			Module.screeps.object.readResourceType($2),
			$3 === -1 ? undefined : $3
		);
	}, &this->id, &target.id, resource, amount);
}

int creep_t::upgrade_controller(const game_object_t& target) const {
	return EM_ASM_INT({
		return Module.screeps.util.getObjectById(Module, $0).upgradeController(Module.screeps.util.getObjectById(Module, $1));
	}, &this->id, &target.id);
}

//int creep_t::withdraw(const game_object_t& target, resource_t resource, int amount = -1) const;

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
		offsetof(creep_t, ticksToLive),
		sizeof(creep_bodypart_t),
		offsetof(creep_bodypart_t, boost),
		offsetof(creep_bodypart_t, type)
	);
}

std::ostream& operator<<(std::ostream& os, const creep_t& that) {
	if (that.my) {
		return os <<"creep_t[" <<that.name <<"]";
	} else {
		return os <<"creep_t[!" <<that.id <<"]";
	}
}

} // namespace screeps
