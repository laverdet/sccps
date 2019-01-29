#pragma once
#include "./array.h"
#include "./constants.h"
#include "./position.h"
#include "./object.h"
#include "./resource.h"
#include "./string.h"
#include "./memory/optional.h"
#include <iosfwd>
#include <optional>
#include <vector>

namespace screeps {

enum struct bodypart_t {
	none,
	attack,
	carry,
	claim,
	heal,
	move,
	ranged_attack,
	tough,
	work,
};
inline constexpr int operator+(bodypart_t part) noexcept {
	return static_cast<int>(part);
}
std::ostream& operator<<(std::ostream& os, bodypart_t type);

struct creep_bodypart_t {
	resource_t boost;
	bodypart_t type;

	template <class Memory>
	void serialize(Memory& memory) {
		memory & boost & type;
	}
};

struct creep_active_bodypart_t {
	const creep_bodypart_t& part;
	int hits;
	creep_active_bodypart_t(const creep_bodypart_t& part, int hits) : part(part), hits(hits) {}
};

struct creep_t : game_object_t {
	using name_t = string_t<20>;
	int32_t fatigue;
	int32_t hits;
	int32_t hits_max;
	int32_t ticks_to_live;
	resource_store_t carry;
	name_t name;
	array_t<creep_bodypart_t, kMaxCreepSize> body;
	// owner
	// saying
	union {
		struct {
			sid_t _spawn_id;
			bool _is_spawning;
		};
		std::optional<sid_t> spawning;
	};
	bool my;

	struct static_method {
		static int build(const sid_t& creep, const sid_t& target);
		static int cancel_order(const sid_t& creep, const char* method);
		static int drop(const sid_t& creep, resource_t resource, int amount = -1);
		static int harvest(const sid_t& creep, const sid_t& target);
		static int move(const sid_t& creep, direction_t direction);
		static int pickup(const sid_t& creep, const sid_t& target);
		static int repair(const sid_t& creep, const sid_t& target);
		static int suicide(const sid_t& creep);
		static int transfer(const sid_t& creep, const sid_t& target, resource_t resource, int amount = -1);
		static int upgrade_controller(const sid_t& creep, const sid_t& target);
		static int withdraw(const sid_t& creep, const sid_t& target, resource_t resource, int amount = -1);
	};

	creep_t() {};

	int build(const game_object_t& target) const {
		return static_method::build(id, target.id);
	}
	int cancel_order(const char* method) const {
		return static_method::cancel_order(id, method);
	}
	int drop(resource_t resource, int amount = -1) const {
		return static_method::drop(id, resource, amount);
	}
	int harvest(const game_object_t& target) const {
		return static_method::harvest(id, target.id);
	}
	int move(direction_t direction) const {
		return static_method::move(id, direction);
	}
	int pickup(const game_object_t& target) const {
		return static_method::pickup(id, target.id);
	}
	int repair(const game_object_t& target) const {
		return static_method::repair(id, target.id);
	}
	int suicide() const {
		return static_method::suicide(id);
	}
	int transfer(const game_object_t& target, resource_t resource, int amount = -1) const {
		return static_method::transfer(id, target.id, resource, amount);
	}
	int upgrade_controller(const game_object_t& target) const {
		return static_method::upgrade_controller(id, target.id);
	}
	int withdraw(const game_object_t& target, resource_t resource, int amount = -1) const {
		return static_method::withdraw(id, target.id, resource, amount);
	}

	const std::vector<creep_active_bodypart_t> get_active_bodyparts() const;

	template <class Memory>
	void serialize(Memory& memory) {
		game_object_t::serialize(memory);
		memory & fatigue & hits & hits_max & ticks_to_live;
		memory & carry & name & body & spawning & my;
	}

	static void init();
	friend std::ostream& operator<<(std::ostream& os, const creep_t& that);
};

} // namespace screeps
