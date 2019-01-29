#pragma once
#include "./position.h"
#include <functional>
#include <iosfwd>

namespace screeps {

// id type
class sid_t {
	public:
		template <class Memory>
		void serialize(Memory& memory) {
			memory & length & bytes;
		}

		// These operators don't check `length` because on a server I figure all ids will be the same
		// length? Maybe?
		constexpr bool operator==(const sid_t& rhs) const {
			return bytes[0] == rhs.bytes[0] && bytes[1] == rhs.bytes[1] && bytes[2] == rhs.bytes[2];
		}
		constexpr bool operator!=(const sid_t& rhs) const { return !(*this == rhs); }
		constexpr bool operator<(const sid_t& rhs) const { return bytes < rhs.bytes; }

		friend std::ostream& operator<<(std::ostream& os, const sid_t& that);

	private:
		uint32_t length;
		uint32_t bytes[3];
		friend struct std::hash<screeps::sid_t>;
};

// Abstract object types
struct room_object_t {
	position_t pos;

	template <class Memory>
	void serialize(Memory& memory) {
		memory & pos;
	}
};

struct game_object_t : room_object_t {
	sid_t id;

	template <class Memory>
	void serialize(Memory& memory) {
		room_object_t::serialize(memory);
		memory & id;
	}

	friend std::ostream& operator<<(std::ostream& os, const game_object_t& that);
};

} // namespace screeps

template <>
struct std::hash<screeps::sid_t> {
	size_t operator()(const screeps::sid_t& id) const {
		return id.bytes[0] ^ id.bytes[1] ^ id.bytes[2];
	}
};
