#include <screeps/constants.h>
#include <screeps/position.h>
#include <screeps/terrain.h>
#include <iostream>

namespace screeps {

std::ostream& operator<<(std::ostream& os, direction_t dir) {
	os <<"direction_t::";
	switch (dir) {
		case direction_t::top: return os <<"top";
		case direction_t::top_right: return os <<"top_right";
		case direction_t::right: return os <<"right";
		case direction_t::bottom_right: return os <<"bottom_right";
		case direction_t::bottom: return os <<"bottom";
		case direction_t::bottom_left: return os <<"bottom_left";
		case direction_t::left: return os <<"left";
		case direction_t::top_left: return os <<"top_left";
	}
	return os <<(int)dir;
}

std::ostream& operator<<(std::ostream& os, room_location_t location) {
	if (location.xx == -0x80 && location.yy == -0x80) {
		os <<"sim";
	} else {
		if (location.xx < 0) {
			os <<'W' <<static_cast<int>(-location.xx - 1);
		} else {
			os <<'E' <<static_cast<int>(location.xx);
		}
		if (location.yy < 0) {
			os <<'N' <<static_cast<int>(-location.yy - 1);
		} else {
			os <<'S' <<static_cast<int>(location.yy);
		}
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, local_position_t position) {
	os <<"local_position_t(" <<static_cast<int>(position.xx) <<", " <<static_cast<int>(position.yy) <<")";
	return os;
}

std::ostream& operator<<(std::ostream& os, position_t position) {
	os <<"position_t(\"" <<position.room <<"\", " <<static_cast<int>(position.xx) <<", " <<static_cast<int>(position.yy) <<")";
	return os;
}

} // namespace screeps
