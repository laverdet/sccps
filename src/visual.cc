#include <screeps/position.h>
#include <screeps/visual.h>
#include <emscripten.h>

namespace screeps {

visual_t::point_t::point_t(local_position_t position) : xx(position.xx), yy(position.yy) {}
visual_t::point_t::point_t(position_t position) : xx(position.to_local().xx), yy(position.to_local().yy) {}

	/*
void visual_t::draw_circle(float xx, float yy, const visual_t::circle_t& options) const {
	EM_ASM({
		Module.screeps.position.getVisual($0, $1).circle($2, $3, {
			'radius': $4,
			'fill': Module.screeps.util.toColor($5),
			'opacity': $6,
			'stroke': Module.screeps.util.toColor($7),
			'strokeWidth': $8,
		});
	},
		this->xx, this->yy, xx, yy,
		options.radius, options.fill.rgba,
		options.opacity,
		options.stroke.rgba, options.stroke_width);
}
*/

void visual_t::draw_line(room_location_t room, point_t p1, point_t p2, const line_t& options) {
	EM_ASM({
		Module.screeps.position.getVisual($0, $1).line(
			$2, $3,
			$4, $5, {
				'width': $6,
				'color': Module.screeps.util.toColor($7),
				'opacity': $8,
				'lineStyle': Module.screeps.util.toLineStyle($9),
			}
		);
	},
		room.xx, room.yy,
		p1.xx, p1.yy,
		p2.xx, p2.yy,
		options.width,
		options.color.rgba,
		options.opacity,
		options.line_style
	);
}

void visual_t::draw_poly(room_location_t room, const std::vector<point_t>& points, const poly_t& options) {
	EM_ASM({
		var points = [];
		for (var ptr = $2; ptr < $3; ptr += 8) {
			points.push([ Module.HEAPF32[ptr >> 2], Module.HEAPF32[(ptr + 4) >> 2] ]);
		}
		Module.screeps.position.getVisual($0, $1).poly(
			points, {
				'fill': Module.screeps.util.toColor($4),
				'opacity': $5,
				'stroke': Module.screeps.util.toColor($6),
				'strokeWidth': $7,
				'lineStyle': Module.screeps.util.toLineStyle($8),
			}
		);
	},
		room.xx, room.yy,
		points.data(), points.data() + points.size(),
		options.fill.rgba,
		options.opacity,
		options.stroke.rgba,
		options.stroke_width,
		options.line_style
	);
}

void visual_t::draw_text(room_location_t room, visual_t::point_t origin, const std::string& text, const visual_t::text_t& options) {
	EM_ASM({
		Module.screeps.position.getVisual($0, $1).text(
			Module.screeps.string.readUtf8StringData(Module, $2, $3),
			$4, $5, {
				'color': Module.screeps.util.toColor($6),
				'font': $8 === 0 ? undefined : Module.screeps.string.readUtf8StringData(Module, $7, $8),
				'stroke': Module.screeps.util.toColor($9),
				'strokeWidth': $10,
				'backgroundColor': Module.screeps.util.toColor($11),
				'backgroundPadding': $12,
				'align': Module.screeps.string.readUtf8StringData(Module, $13, $14),
				'opacity': $15,
			}
		);
	},
		room.xx, room.yy,
		text.c_str(), text.length(),
		origin.xx, origin.yy,
		options.color.rgba,
		options.font.c_str(), options.font.length(),
		options.stroke.rgba, options.stroke_width,
		options.background_color.rgba, options.background_padding,
		options.align.c_str(), options.align.length(),
		options.opacity
	);
}

void visual_t::draw_text(position_t position, const std::string& text, const visual_t::text_t& options) {
	visual_t::draw_text(position.room_location(), position, text, options);
}

} // screeps
