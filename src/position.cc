#include <screeps/constants.h>
#include <screeps/position.h>
#include <screeps/terrain.h>
#include <emscripten.h>

namespace screeps {

const terrain_t& room_location_t::terrain() const {
	static terrain_t* terrain_map[room_location_t::max]{nullptr};
	if (terrain_map[id] == nullptr) {
		terrain_map[id] = new terrain_t(*this);
	}
	return *terrain_map[id];
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
