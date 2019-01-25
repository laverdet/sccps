#include "./javascript.h"
#include <screeps/flag.h>

namespace screeps {

void flag_t::init() {
	EM_ASM({
		Module.screeps.object.initFlagLayout({
			'sizeof': $0,
			'color': $1,
			'secondaryColor': $2,
		});
	},
		sizeof(flag_t),
		offsetof(flag_t, color),
		offsetof(flag_t, secondary_color),
	);
}

void flag_t::remove() const {
	std::cerr <<*this <<".remove()\n";
}

void flag_t::set_color(color_t color, color_t secondary_color) {
	std::cerr <<*this <<".set_color(" <<color << ", " <<secondary_color <<")\n";
}

} // namespace screeps
