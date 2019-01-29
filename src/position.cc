#include <screeps/constants.h>
#include <screeps/position.h>
#include <screeps/terrain.h>
#include <iostream>

namespace screeps {

const terrain_t& room_location_t::terrain(terrain_t* terrain) const {
	static terrain_t* terrain_map[256 * 256]{nullptr};
	int id = xx * 256 + yy;
	if (terrain_map[id] == nullptr) {
		terrain_map[id] = terrain == nullptr ? new terrain_t(*this) : terrain;
	}
	return *terrain_map[id];
}

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

std::ostream& operator<<(std::ostream& os, room_location_t that) {
	if (that.id == 0) {
		os <<"sim";
	} else {
		if (that.xx < k_world_size2) {
			os <<'W' <<k_world_size2 - that.xx - 1;
		} else {
			os <<'E' <<that.xx - k_world_size2;
		}
		if (that.yy < k_world_size2) {
			os <<'N' <<k_world_size2 - that.yy - 1;
		} else {
			os <<'S' <<that.yy - k_world_size2;
		}
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, local_position_t that) {
	os <<"local_position_t[" <<(int)that.xx <<", " <<(int)that.yy <<"]";
	return os;
}

std::ostream& operator<<(std::ostream& os, position_t that) {
	os <<"position_t[" <<that.room_location() <<", " <<that.xx % 50 <<", " <<that.yy % 50 <<"]";
	return os;
}

} // namespace screeps
