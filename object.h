#pragma once
#include "./constants.h"
#include "./position.h"
#include "./string.h"

// Abstract object types
struct room_object_t {
	const position_t pos;
};

using id_t = string_t<kMaximumIdLength>;
struct game_object_t : room_object_t {
	const id_t id;
};
