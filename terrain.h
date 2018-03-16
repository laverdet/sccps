#pragma once
#include <cstdint>
#include "./position.h"

// TODO: local_matrix_t
class terrain_t {
	private:
		uint8_t data[625];

	public:
		static constexpr uint8_t plain = 0;
		static constexpr uint8_t wall = 1;
		static constexpr uint8_t swamp = 2;
		terrain_t() : data {0} {}
		terrain_t(room_location_t room);

		uint8_t get(uint8_t xx, uint8_t yy) const {
			uint16_t index = xx * 50 + yy;
			return (data[index >> 2] >> ((index & 3) << 1)) & 3;
		}

		uint8_t get(local_position_t pos) const {
			return get(pos.xx, pos.yy);
		}

		void set(uint8_t xx, uint8_t yy, uint8_t val) {
			uint16_t index = xx * 50 + yy;
			uint8_t bit_pos = (index & 3) << 1;
			index >>= 2;
			data[index] = (data[index] & ~(3 << bit_pos)) | val << bit_pos;
		}

		void set(local_position_t pos, uint8_t val) {
			set(pos.xx, pos.yy, val);
		}
};
