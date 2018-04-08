#pragma once
#include "./array.h"
#include "./constants.h"
#include "./position.h"
#include "./object.h"
#include "./resource.h"
#include <vector>

namespace screeps {

enum struct bodypart_t {
	attack,
	carry,
	claim,
	heal,
	move,
	ranged_attack,
	tough,
	work,
};

using creep_body_t = array_t<bodypart_t, kMaxCreepSize>;

struct creep_bodypart_t {
	resource_t boost;
	bodypart_t type;
};

struct creep_active_bodypart_t {
	const creep_bodypart_t& part;
	uint8_t hits;
	creep_active_bodypart_t() = default;
	creep_active_bodypart_t(const creep_bodypart_t& part, uint8_t hits) : part(part), hits(hits) {}
};

struct creep_t : game_object_t {
	using name_t = string_t<20>;

	array_t<creep_bodypart_t, kMaxCreepSize> body;
	resource_store_t carry;
	uint32_t carry_capacity;
	uint32_t fatigue;
	uint32_t hits;
	uint32_t hits_max;
	bool my;
	name_t name;
	// owner
	// saying
	bool spawning;
	uint32_t ticksToLive;

	int build(const game_object_t& target) const;
	int drop(resource_t resource, int amount) const;
	int harvest(const game_object_t& target) const;
	int move(direction_t direction) const;
	int pickup(const game_object_t& target) const;
	int repair(const game_object_t& target) const;
	int suicide() const;
	int transfer(const game_object_t& target, resource_t resource, int amount = -1) const;
	int upgrade_controller(const game_object_t& target) const;
	int withdraw(const game_object_t& target, resource_t resource, int amount = -1) const;

	const std::vector<creep_active_bodypart_t> get_active_bodyparts() const;

	static void init();
};

} // namespace screeps
