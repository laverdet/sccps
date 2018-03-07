#pragma once
#include <iostream>
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
	friend std::ostream& operator<< (std::ostream& os, const room_location_t& that);
};

// This represents a position in a continuous plane of the whole world. `roomName` is implied from
// xx and yy.
struct position_t {
	uint16_t xx, yy;

	room_location_t room_location() const;
	friend std::ostream& operator<< (std::ostream& os, const position_t& that);
};
