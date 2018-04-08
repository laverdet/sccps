#include "./constants.h"
#include "./position.h"
#include "./terrain.h"
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

void room_location_t::draw_circle(double xx, double yy, const room_location_t::circle_t& options) const {
	EM_ASM({
		Module.screeps.position.getVisual($0, $1).circle($2, $3, {
			'radius': $4,
			'fill': '#' + Module.screeps.util.pad(($5).toString(16), 6, '0'),
			'opacity': $6,
			'stroke': $7 === -1 ? undefined : '#' + Module.screeps.util.pad(($7).toString(16), 6, '0'),
			'strokeWidth': $8,
		});
	}, this->xx, this->yy, xx, yy, options.radius, options.fill, options.opacity, options.stroke, options.stroke_width);
}

void room_location_t::draw_text(double xx, double yy, const std::string& text, const room_location_t::text_t& options) const {
	EM_ASM({
		Module.screeps.position.getVisual($0, $1).text(
			Module.screeps.string.readOneByteStringData(Module, $2, $3),
			$4, $5, {
				'color': '#' + Module.screeps.util.pad(($6).toString(16), 6, '0'),
				'font': $8 === 0 ? undefined : Module.screeps.string.readOneByteStringData(Module, $7, $8),
				'stroke': $9 === -1 ? undefined : '#' + Module.screeps.util.pad(($9).toString(16), 6, '0'),
				'strokeWidth': $10,
				'backgroundColor': $11 === -1 ? undefined : '#' + Module.screeps.util.pad(($11).toString(16), 6, '0'),
				'backgroundPadding': $12,
				'align': Module.screeps.string.readOneByteStringData(Module, $13, $14),
				'opacity': $15,
			}
		);
	},
		this->xx, this->yy,
		text.c_str(), text.length(),
		xx, yy,
		options.color,
		options.font.c_str(), options.font.length(),
		options.stroke, options.stroke_width,
		options.background_color, options.background_padding,
		options.align.c_str(), options.align.length(),
		options.opacity
	);
}

std::ostream& operator<<(std::ostream& os, local_position_t that) {
	os <<"local_position_t[" <<(int)(that.xx % 50) <<", " <<(int)(that.yy % 50) <<"]";
	return os;
}

std::ostream& operator<<(std::ostream& os, position_t that) {
	os <<"position_t[" <<that.room_location() <<", " <<that.xx % 50 <<", " <<that.yy % 50 <<"]";
	return os;
}

} // namespace screeps
