#include "./position.h"
#include "./terrain.h"
#include <emscripten.h>

const terrain_t& room_location_t::terrain() const {
	static terrain_t* terrain_map[room_location_t::max]{nullptr};
	if (terrain_map[id] == nullptr) {
		terrain_map[id] = new terrain_t(*this);
	}
	return *terrain_map[id];
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

void room_location_t::draw_circle(double xx, double yy, const room_location_t::circle_t& options) const {
	EM_ASM({
		Module.screeps.position.getVisual($0, $1).circle($2, $3, {
			'radius': $4,
			'fill': '#' + Module.screeps.util.pad($5, 6, 'f'),
			'opacity': $6,
			'stroke': $7 === 0xff000000 ? undefined : '#' + Module.screeps.util.pad($7, 6, 'f'),
			'strokeWidth': $8,
		});
	}, this->xx, this->yy, xx, yy, options.radius, options.fill, options.opacity, options.stroke, options.stroke_width);
}

std::ostream& operator<< (std::ostream& os, const local_position_t& that) {
	os <<"local_position_t[" <<(int)(that.xx % 50) <<", " <<(int)(that.yy % 50) <<"]";
	return os;
}
std::ostream& operator<< (std::ostream& os, const position_t& that) {
	os <<"position_t[" <<that.room_location() <<", " <<that.xx % 50 <<", " <<that.yy % 50 <<"]";
	return os;
}
