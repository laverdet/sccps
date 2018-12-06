#include <screeps/constants.h>
#include <screeps/position.h>
#include <screeps/terrain.h>

namespace screeps {

const terrain_t& room_location_t::terrain() const {
	static terrain_t* terrain_map[256 * 256]{nullptr};
	int id = xx * 256 + yy;
	if (terrain_map[id] == nullptr) {
		terrain_map[id] = new terrain_t(*this);
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
	if (that.xx == 0 && that.yy == 0) {
		os <<"sim";
	} else {
		bool w = that.xx <= kWorldSize >> 1;
		bool n = that.yy <= kWorldSize >> 1;
		os <<(w ? 'W' : 'E')
			<<(w ? 127 - that.xx : that.xx - 128)
			<<(n ? 'N' : 'S')
			<<(n ? 127 - that.yy : that.yy - 128);
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
