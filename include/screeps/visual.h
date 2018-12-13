#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace screeps {

struct room_location_t;
struct local_position_t;
struct position_t;

struct _visual_structs_t {
	// Workaround for following clang error:
	// "error: default member initializer for 'width' needed within definition of enclosing class 'visual_t' outside of member functions"
	// Defining these structures outside the struct they're used in fixes it.
	struct color_t {
		uint32_t rgba;
		color_t() : rgba(0) {}
		color_t(int rgb) : rgba(0xff000000 | rgb & 0xffffff) {}
		color_t(int rgb, int opacity) : rgba((opacity & 0xff) << 24 | (rgb & 0xffffff)) {}
	};

	enum struct line_style_t { solid, dashed, dotted };

	struct line_t {
		float width = 0.1;
		color_t color = 0xffffff;
		float opacity = 0.5;
		line_style_t line_style = line_style_t::solid;
	};

	struct poly_t {
		color_t fill = 0xffffff;
		float opacity = 0.5;
		color_t stroke{};
		float stroke_width = 0.1;
		line_style_t line_style = line_style_t::solid;
	};

	struct text_t {
		color_t color = 0xffffff;
		std::string font = "";
		color_t stroke{};
		float stroke_width = 0.15;
		color_t background_color{};
		float background_padding = 0.3;
		std::string align = "center";
		float opacity = 1;
	};
};

struct visual_t {
	using color_t = _visual_structs_t::color_t;
	using line_style_t = _visual_structs_t::line_style_t;
	using circle_t = _visual_structs_t::poly_t;
	using line_t = _visual_structs_t::line_t;
	using poly_t = _visual_structs_t::poly_t;
	using rect_t = _visual_structs_t::poly_t;
	using text_t = _visual_structs_t::text_t;

	struct point_t {
		float xx, yy;
		point_t(local_position_t position);
		point_t(position_t position);
		constexpr point_t(float xx, float yy) : xx(xx), yy(yy) {}
		constexpr point_t operator+(point_t that) const {
			return point_t(xx + that.xx, yy + that.yy);
		}
		constexpr point_t operator-(point_t that) const {
			return point_t(xx - that.xx, yy - that.yy);
		}
		constexpr point_t& operator+=(point_t that) {
			xx += that.xx;
			yy += that.yy;
			return *this;
		}
		constexpr point_t& operator-=(point_t that) {
			xx -= that.xx;
			yy -= that.yy;
			return *this;
		}
	};

	static void draw_circle(room_location_t room, point_t center, float radius = 0.15, const circle_t& options = {});
	static void draw_circle(position_t position, float radius = 0.15, const circle_t& options = {});
	static void draw_line(room_location_t room, point_t p1, point_t p2, const line_t& options = {});
	static void draw_poly(room_location_t room, const std::vector<point_t>& points, const poly_t& options = {});
	static void draw_poly(room_location_t room, point_t pos, std::vector<point_t> points, const poly_t& options = {});
	static void draw_poly(position_t pos, const std::vector<point_t>& points, const poly_t& options = {});
	static void draw_rect(room_location_t room, point_t p1, float width, float height, const rect_t& options = {});
	static void draw_rect(room_location_t room, point_t p1, point_t p2, const rect_t& options = {});
	static void draw_rect(position_t pos, float width, float height, const rect_t& options = {});
	static void draw_text(room_location_t room, point_t origin, const std::string& text, const text_t& options = {});
	static void draw_text(position_t position, const std::string& text, const text_t& options = {});
};

} // namespace screeps
