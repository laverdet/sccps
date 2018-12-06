#include <screeps/position.h>
#include <screeps/visual.h>

namespace screeps {

void visual_t::draw_circle(room_location_t room, point_t center, float radius, const visual_t::circle_t& options) {
}

void visual_t::draw_line(room_location_t room, point_t p1, point_t p2, const line_t& options) {
}

void visual_t::draw_poly(room_location_t room, const std::vector<point_t>& points, const poly_t& options) {
}

void visual_t::draw_rect(room_location_t room, point_t pos, float width, float height, const rect_t& options) {
}

void visual_t::draw_text(room_location_t room, visual_t::point_t origin, const std::string& text, const visual_t::text_t& options) {
}

} // screeps
