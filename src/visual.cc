#include <screeps/position.h>
#include <screeps/visual.h>
#include "./javascript.h"

namespace screeps {

visual_t::point_t::point_t(local_position_t position) : xx(position.xx), yy(position.yy) {}
visual_t::point_t::point_t(position_t position) : xx(position.to_local().xx), yy(position.to_local().yy) {}

void visual_t::draw_circle(position_t position, float radius, const visual_t::circle_t& options) {
	visual_t::draw_circle(position.room_location(), position, radius, options);
}

void visual_t::draw_poly(room_location_t room, point_t pos, std::vector<point_t> points, const poly_t& options) {
	for (auto& ii : points) {
		ii += pos;
	}
	draw_poly(room, points, options);
}

void visual_t::draw_poly(position_t pos, const std::vector<point_t>& points, const poly_t& options) {
	draw_poly(pos.room_location(), pos.to_local(), points, options);
}

void visual_t::draw_rect(room_location_t room, point_t p1, point_t p2, const rect_t& options) {
	draw_rect(room, p1, p2.xx - p1.xx, p2.yy - p1.yy, options);
}

void visual_t::draw_rect(position_t pos, float width, float height, const rect_t& options) {
	draw_rect(pos.room_location(), pos.to_local(), width, height, options);
}

void visual_t::draw_text(position_t position, const std::string& text, const visual_t::text_t& options) {
	visual_t::draw_text(position.room_location(), position, text, options);
}

#ifdef JAVASCRIPT
void visual_t::draw_circle(room_location_t room, point_t center, float radius, const visual_t::circle_t& options) {
	EM_ASM({
		Module.screeps.position.getVisual($0).circle(
			$1, $2, {
				'radius': $3,
				'fill': Module.screeps.util.toColor($4),
				'opacity': $5,
				'stroke': Module.screeps.util.toColor($6),
				'strokeWidth': $7,
				'lineStyle': Module.screeps.util.toLineStyle($8),
			}
		);
	},
		room.id,
		center.xx, center.yy,
		radius, options.fill.rgba,
		options.opacity,
		options.stroke.rgba,
		options.stroke_width,
		options.line_style
	);
}

void visual_t::draw_line(room_location_t room, point_t p1, point_t p2, const line_t& options) {
	EM_ASM({
		Module.screeps.position.getVisual($0).line(
			$1, $2,
			$3, $4, {
				'width': $5,
				'color': Module.screeps.util.toColor($6),
				'opacity': $7,
				'lineStyle': Module.screeps.util.toLineStyle($8),
			}
		);
	},
		room.id,
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
		for (var ptr = $1; ptr != $2; ptr += 8) {
			points.push([ Module.readFloat32(ptr), Module.readFloat32(ptr + 4) ]);
		}
		Module.screeps.position.getVisual($0).poly(
			points, {
				'fill': Module.screeps.util.toColor($3),
				'opacity': $4,
				'stroke': Module.screeps.util.toColor($5) || 'transparent',
				'strokeWidth': $6,
				'lineStyle': Module.screeps.util.toLineStyle($7),
			}
		);
	},
		room.id,
		points.data(),
		points.data() + points.size(),
		options.fill.rgba,
		options.opacity,
		options.stroke.rgba,
		options.stroke_width,
		options.line_style
	);
}

void visual_t::draw_rect(room_location_t room, point_t pos, float width, float height, const rect_t& options) {
	EM_ASM({
		Module.screeps.position.getVisual($0).rect(
			$1, $2,
			$3, $4, {
				'fill': Module.screeps.util.toColor($5),
				'opacity': $6,
				'stroke': Module.screeps.util.toColor($7),
				'strokeWidth': $8,
				'lineStyle': Module.screeps.util.toLineStyle($9),
			}
		);
	},
		room.id,
		pos.xx, pos.yy,
		width, height,
		options.fill.rgba,
		options.opacity,
		options.stroke.rgba,
		options.stroke_width,
		options.line_style
	);
}

void visual_t::draw_text(room_location_t room, visual_t::point_t origin, const std::string& text, const visual_t::text_t& options) {
	EM_ASM({
		Module.screeps.position.getVisual($0).text(
			Module.screeps.string.readUtf8StringData(Module, $1, $2),
			$3, $4, {
				'color': Module.screeps.util.toColor($5),
				'font': $7 === 0 ? undefined : Module.screeps.string.readUtf8StringData(Module, $6, $7),
				'stroke': Module.screeps.util.toColor($8),
				'strokeWidth': $9,
				'backgroundColor': Module.screeps.util.toColor($10),
				'backgroundPadding': $11,
				'align': Module.screeps.string.readUtf8StringData(Module, $12, $13),
				'opacity': $14,
			}
		);
	},
		room.id,
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
#else
void visual_t::draw_circle(room_location_t /* room */, point_t /* center */, float /* radius */, const visual_t::circle_t& /* options */) {
}

void visual_t::draw_line(room_location_t /* room */, point_t /* p1 */, point_t /* p2 */, const line_t& /* options */) {
}

void visual_t::draw_poly(room_location_t /* room */, const std::vector<point_t>& /* points */, const poly_t& /* options */) {
}

void visual_t::draw_rect(room_location_t /* room */, point_t /* pos */, float /* width */, float /* height */, const rect_t& /* options */) {
}

void visual_t::draw_text(room_location_t /* room */, visual_t::point_t /* origin */, const std::string& /* text */, const visual_t::text_t& /* options */) {
}
#endif

} // namespace screeps
