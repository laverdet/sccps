#pragma once
#include <iostream>
#include <unordered_map>
constexpr int kWorldSize = 255;

// These will match the constants in game
enum struct direction_t { top = 1, top_right, right, bottom_right, bottom, bottom_left, left, top_left };

// This is replacement for `roomName` in the JS API.
// "E0S0" -> { xx: 128, yy: 128 }
struct room_location_t {
	union {
		uint16_t id;
		struct {
			uint8_t xx, yy;
		};
	};
	room_location_t(uint16_t xx, uint16_t yy) : xx(xx), yy(yy) {}
	room_location_t(const room_location_t& that) : xx(that.xx), yy(that.yy) {}

	const class terrain_t& get_terrain() const;

	friend std::ostream& operator<< (std::ostream& os, const room_location_t& that);

	bool operator==(const room_location_t& rhs) const {
		return id == rhs.id;
	}
};

// Hash specialization for room_location_t
template <> struct std::hash<room_location_t> {
	size_t operator()(const room_location_t& val) const {
		return std::hash<uint16_t>()(val.id);
	}
};

// This represents a position in a continuous plane of the whole world. `roomName` is implied from
// xx and yy.
struct position_t {
	union {
		uint32_t id;
		struct {
			uint16_t xx, yy;
		};
	};
	position_t() = default;
	position_t(uint16_t xx, uint16_t yy) : xx(xx), yy(yy) {}

	room_location_t room_location() const;

	friend std::ostream& operator<< (std::ostream& os, const position_t& that);

	bool operator==(const position_t& rhs) const {
		return id == rhs.id;
	}
};
