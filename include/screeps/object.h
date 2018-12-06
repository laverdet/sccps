#pragma once
#include <iostream>
#include "./constants.h"
#include "./position.h"
#include "./string.h"

namespace screeps {

// Abstract object types
struct room_object_t {
	position_t pos;
};

using sid_t = string_t<kMaximumIdLength>;
struct game_object_t : room_object_t {
	sid_t id;
	friend std::ostream& operator<<(std::ostream& os, const game_object_t& that);
};

} // namespace screeps
