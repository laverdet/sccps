#include "./position.h"

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
