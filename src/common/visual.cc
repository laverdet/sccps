#include <screeps/position.h>
#include <screeps/visual.h>

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

} // namespace screeps
