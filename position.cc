#include "./position.h"
#include "./terrain.h"

static std::unordered_map<room_location_t, terrain_t> terrain_map;

const terrain_t& room_location_t::get_terrain() const {
	auto ii = terrain_map.find(*this);
	if (ii == terrain_map.end()) {
		ii = terrain_map.emplace(std::piecewise_construct, std::forward_as_tuple(*this), std::forward_as_tuple()).first;
		ii->second.load(*this);
	}
	return ii->second;
}

std::ostream& operator<< (std::ostream& os, const room_location_t& that) {
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

room_location_t position_t::room_location() const {
	return room_location_t(xx / 50, yy / 50);
};

std::ostream& operator<< (std::ostream& os, const position_t& that) {
	os <<"position_t[" <<that.room_location() <<"] " <<that.xx % 50 <<", " <<that.yy % 50 <<")";
	return os;
}
