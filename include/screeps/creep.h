#pragma once
#include "./array.h"
#include "./constants.h"
#include "./position.h"
#include "./object.h"
#include "./resource.h"
#include <iostream>
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
	int hits;
	creep_active_bodypart_t() = default;
	creep_active_bodypart_t(const creep_bodypart_t& part, int hits) : part(part), hits(hits) {}
};

struct creep_t : game_object_t {
	using name_t = string_t<20>;
	int32_t carry_capacity;
	int32_t fatigue;
	int32_t hits;
	int32_t hits_max;
	int32_t ticksToLive;
	resource_store_t carry;
	name_t name;
	array_t<creep_bodypart_t, kMaxCreepSize> body;
	// owner
	// saying
	bool spawning;
	bool my;

	int build(const game_object_t& target) const;
	int cancel_order(const char* method) const;
	int drop(resource_t resource, int amount = -1) const;
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
	friend std::ostream& operator<<(std::ostream& os, const creep_t& that);
};

} // namespace screeps
