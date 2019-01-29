#include "./javascript.h"
#include <screeps/flag.h>

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

} // namespace screeps
