#pragma once
#include <cstdint>
#include "./position.h"

namespace screeps {

class terrain_t : public local_matrix_t<uint8_t, uint8_t, 2> {
	public:
		static constexpr uint8_t plain = 0;
		static constexpr uint8_t wall = 1;
		static constexpr uint8_t swamp = 2;
		terrain_t() = default;
		explicit terrain_t(room_location_t room);
};

} // namespace screeps
