#include "./javascript.h"
#include <screeps/flag.h>
#include <iostream>

namespace screeps {

void flag_t::init() {
	EM_ASM({
		Module.screeps.object.initFlagLayout({
			'sizeof': $0,
			'name': $1,
			'color': $2,
			'secondaryColor': $3,
		});
	},
		sizeof(flag_t),
		offsetof(flag_t, name),
		offsetof(flag_t, color),
		offsetof(flag_t, secondary_color)
	);
}

#ifdef JAVASCRIPT
void flag_t::remove() const {
	EM_ASM({
		Module.screeps.string.readOneByteString(Module, $0).remove();
	}, &this->name);
}

void flag_t::set_color(color_t color, color_t secondary_color) const {
	EM_ASM({
		Module.screeps.string.readOneByteString(Module, $0).setColor(Module.screeps.object.readColor($1), Module.screeps.object.readColor($2));
	}, &this->name, color, secondary_color);
}
#else
void flag_t::remove() const {
	std::cerr <<name <<".remove()\n";
}

void flag_t::set_color(color_t color, color_t secondary_color) const {
	std::cerr <<name <<".set_color(" <<color << ", " <<secondary_color <<")\n";
}
#endif

std::ostream& operator<<(std::ostream& os, color_t color) {
	os <<"color_t::";
	switch (color) {
		case color_t::blue: return os <<"blue";
		case color_t::brown: return os <<"brown";
		case color_t::cyan: return os <<"cyan";
		case color_t::green: return os <<"green";
		case color_t::grey: return os <<"grey";
		case color_t::orange: return os <<"orange";
		case color_t::purple: return os <<"purple";
		case color_t::red: return os <<"red";
		case color_t::white: return os <<"white";
		case color_t::yellow: return os <<"yellow";
	}
}

} // namespace screeps
