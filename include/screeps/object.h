#pragma once
#include <iosfwd>
#include "./constants.h"
#include "./position.h"
#include "./string.h"

namespace screeps {

// Abstract object types
struct room_object_t {
	position_t pos;

	template <class Memory>
	void serialize(Memory& memory) {
		memory & pos;
	}
};

using sid_t = string_t<kMaximumIdLength>;
struct game_object_t : room_object_t {
	sid_t id;

	template <class Memory>
	void serialize(Memory& memory) {
		room_object_t::serialize(memory);
		memory & id;
	}

	friend std::ostream& operator<<(std::ostream& os, const game_object_t& that);
};

} // namespace screeps
