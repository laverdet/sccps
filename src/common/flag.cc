#include <screeps/flag.h>
#include <iostream>

namespace screeps {

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
