#pragma once
#include "./position.h"
#include <cstdint>
#include <memory>

namespace screeps {

class terrain_t : public local_matrix_t<uint8_t, uint8_t, 2> {
	friend class game_state_t;
	public:
		static constexpr uint8_t plain = 0;
		static constexpr uint8_t wall = 1;
		static constexpr uint8_t swamp = 2;
		terrain_t() = default;
		static void flush();
		static std::shared_ptr<terrain_t> load(room_location_t room);

	protected:
		explicit terrain_t(room_location_t room);
		static void insert(room_location_t room, std::shared_ptr<terrain_t> terrain);
};

} // namespace screeps
