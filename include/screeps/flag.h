#pragma once
#include "./object.h"
#include "./string.h"

namespace screeps {

enum class color_t {
	blue,
	brown,
	cyan,
	green,
	grey,
	orange,
	purple,
	red,
	white,
	yellow,
	last = yellow
};
std::ostream& operator<<(std::ostream& os, color_t dir);

struct flag_t : room_object_t {
	using name_t = string_t<kMaxFlagNameLength>;
	name_t name;
	color_t color;
	color_t secondary_color;
	void remove() const;
	void set_color(color_t color, color_t secondary_color = static_cast<color_t>(-1)) const;
	friend class game_state_t;
	private: static void init();
};

} // namespace screeps
