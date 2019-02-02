#pragma once
#include "./constants.h"
#include "./iterator.h"
#include <climits>
#include <array>
#include <iosfwd>
#include <limits>
#include <string_view>
#include <unordered_map>

namespace screeps {

// Forward declarations
enum struct direction_t;
struct room_location_t;
struct position_t;
struct local_position_t;
struct world_position_t;

namespace detail {
// Converts coordinate objects into raw bytes. All location objects are <= word size so we can use
// this for comparisons and hashing.
template <class Type>
constexpr std::enable_if_t<sizeof(Type) == 2, int> flatten(Type location) {
	union union_t {
		constexpr explicit union_t(Type location) : location(location) {}
		Type location;
		uint16_t integer;
	};
	return union_t(location).integer;
}

template <class Type>
constexpr std::enable_if_t<sizeof(Type) == 4, int> flatten(Type location) {
	union union_t {
		constexpr explicit union_t(Type location) : location(location) {}
		Type location;
		int32_t integer;
	};
	return union_t(location).integer;
}
} // namespace detail

//
// Directional constants.. these match the constants in JS
enum struct direction_t { top = 1, top_right, right, bottom_right, bottom, bottom_left, left, top_left };

//
// This is replacement for `roomName` in the JS API. "W0S1" -> { xx: -1, yy: 1 }
struct alignas(int16_t) room_location_t {
	int8_t xx;
	int8_t yy;

	room_location_t() = default;
	constexpr room_location_t(int xx, int yy) : xx(xx), yy(yy) {}
	constexpr room_location_t(std::string_view room_name); // NOLINT(hicpp-explicit-conversions)

	template <class Memory> void serialize(Memory& memory) { memory & xx & yy; }
	friend std::ostream& operator<<(std::ostream& os, room_location_t that);

	constexpr bool operator==(room_location_t rhs) const { return detail::flatten(*this) == detail::flatten(rhs); }
	constexpr bool operator!=(room_location_t rhs) const { return !(*this == rhs); }
	constexpr bool operator<(room_location_t rhs) const { return detail::flatten(*this) < detail::flatten(rhs); }
	constexpr bool operator>(room_location_t rhs) const { return rhs < *this; }
	constexpr bool operator<=(room_location_t rhs) const { return !(*this > rhs); }
	constexpr bool operator>=(room_location_t rhs) const { return !(*this < rhs); }
	constexpr struct position_t operator[](struct local_position_t pos) const;

	static const room_location_t null;
};
inline const room_location_t room_location_t::null = room_location_t(0x80, 0x7f);

//
// Unambiguous representation of a position on a single shard
struct alignas(int32_t) position_t {
	int8_t xx;
	int8_t yy;
	room_location_t room;

	position_t() = default;
	constexpr position_t(room_location_t room, int xx, int yy) : xx(xx), yy(yy), room(room) {}
	constexpr position_t(room_location_t room, local_position_t pos);
	constexpr position_t(world_position_t pos); // NOLINT(hicpp-explicit-conversions)

	template <class Memory> void serialize(Memory& memory) { memory & xx & yy & room; }
	friend std::ostream& operator<<(std::ostream& os, position_t that);

	constexpr bool operator==(position_t rhs) const { return detail::flatten(*this) == detail::flatten(rhs); }
	constexpr bool operator!=(position_t rhs) const { return !(*this == rhs); }
	constexpr bool operator<(position_t rhs) const { return detail::flatten(*this) < detail::flatten(rhs); }
	constexpr bool operator>(position_t rhs) const { return rhs < *this; }
	constexpr bool operator<=(position_t rhs) const { return !(*this > rhs); }
	constexpr bool operator>=(position_t rhs) const { return !(*this < rhs); }
	constexpr local_position_t operator~() const;

	constexpr direction_t direction_to(position_t that) const;
	constexpr position_t in_direction(direction_t direction) const;
	constexpr bool near_to(position_t that) const;
	constexpr int range_to(position_t that) const;
	constexpr int range_to_edge() const;

	constexpr auto neighbors() const;

	static const position_t null;
};
inline const position_t position_t::null = position_t(room_location_t::null, 0xff, 0xff);

//
// Simple container for a location in an arbitrary room
struct alignas(int16_t) local_position_t {
	int8_t xx;
	int8_t yy;

	local_position_t() = default;
	constexpr local_position_t(int xx, int yy) : xx(xx), yy(yy) {}

	template <class Memory> void serialize(Memory& memory) { memory & xx & yy; }
	friend std::ostream& operator<<(std::ostream& os, local_position_t that);

	constexpr bool operator==(local_position_t rhs) const { return detail::flatten(*this) == detail::flatten(rhs); }
	constexpr bool operator!=(local_position_t rhs) const { return !(*this == rhs); }
	constexpr bool operator<(local_position_t rhs) const { return detail::flatten(*this) < detail::flatten(rhs); }
	constexpr bool operator>(local_position_t rhs) const { return rhs < *this; }
	constexpr bool operator<=(local_position_t rhs) const { return !(*this > rhs); }
	constexpr bool operator>=(local_position_t rhs) const { return !(*this < rhs); }

	constexpr direction_t direction_to(local_position_t that) const;
	constexpr local_position_t in_direction(direction_t direction) const;
	constexpr int range_to(local_position_t that) const;
	constexpr int range_to_edge() const;

	constexpr auto neighbors() const;
	constexpr auto with_range(int range) const;
	constexpr auto within_range(int range) const;
	template <class Container>
	constexpr auto within_range(int range, Container&& container) const;
	static constexpr auto all();
	static constexpr auto area(local_position_t top_left, local_position_t bottom_right);

	static const local_position_t null;
};
inline const local_position_t local_position_t::null = local_position_t(0xff, 0xff);

// Similar to `position_t` except the accessors cross room borders. Primarily useful if you're doing
// custom pathfinding stuff.
struct alignas(int32_t) world_position_t {
	uint16_t xx;
	uint16_t yy;

	world_position_t() = default;
	constexpr world_position_t(unsigned xx, unsigned yy) : xx(xx), yy(yy) {}
	constexpr world_position_t(position_t position); // NOLINT(hicpp-explicit-conversions)

	constexpr direction_t direction_to(world_position_t that) const;
	constexpr world_position_t in_direction(direction_t direction) const;
	constexpr int range_to(world_position_t that) const;

	constexpr auto neighbors() const;

	constexpr room_location_t room_location() const {
		return {static_cast<int8_t>(xx / 50u - 0x80), static_cast<int8_t>(yy / 50u - 0x80)};
	}

	static const world_position_t null;
};
inline const world_position_t world_position_t::null = world_position_t(UINT_MAX, UINT_MAX);

//
// Implementation details follow
namespace detail {

// Standard abs is not constexpr
constexpr int abs(int value) {
	return value < 0 ? -value : value;
}

// I think this is an interview question.
constexpr const char* atoi(const char* begin, const char* end, int8_t& result) {
	result = 0;
	if (begin == end || *begin < '0' || *begin > '9') {
		throw std::invalid_argument("Invalid room name");
	}
	do {
		int digit = *begin - '0';
		if (digit >= 0 && digit <= 9) {
			result = result * 10 + digit;
			++begin;
		} else {
			break;
		}
	} while (begin != end);
	return begin;
}

template <class Position>
constexpr direction_t direction_to(Position first, Position second) {
	if (first.xx == second.xx) {
		if (first.yy == second.yy) {
			throw std::logic_error("`direction_to` same position");
		} else if (first.yy < second.yy) {
			return direction_t::bottom;
		} else {
			return direction_t::top;
		}
	} else if (first.yy == second.yy) {
		if (first.xx < second.xx) {
			return direction_t::right;
		} else {
			return direction_t::left;
		}
	} else if (first.xx < second.xx) {
		if (first.yy < second.yy) {
			return direction_t::bottom_right;
		} else {
			return direction_t::top_right;
		}
	} else if (first.yy < second.yy) {
		return direction_t::bottom_left;
	} else {
		return direction_t::top_left;
	}
}

// Used by `local_position_t` and `world_position_t`
template <class Position>
constexpr Position in_direction(Position position, direction_t direction) {
	auto one = std::conditional_t<
		std::is_signed_v<decltype(position.xx)>,
		std::integral_constant<int, 1>,
		std::integral_constant<unsigned, 1u>
	>::value;
	switch (direction) {
		case direction_t::top:
			return {position.xx, position.yy - one};
		case direction_t::top_right:
			return {position.xx + one, position.yy - one};
		case direction_t::right:
			return {position.xx + one, position.yy};
		case direction_t::bottom_right:
			return {position.xx + one, position.yy + one};
		case direction_t::bottom:
			return {position.xx, position.yy + one};
		case direction_t::bottom_left:
			return {position.xx - one, position.yy + one};
		case direction_t::left:
			return {position.xx - one, position.yy};
		case direction_t::top_left:
			return {position.xx - one, position.yy - one};
		default:
			throw std::invalid_argument("Invalid direction");
	}
}

// Generates a range of directions by template stored indefinitely and returns them
template <direction_t... Directions>
inline std::pair<const direction_t*, const direction_t*> make_direction_range() {
	static constexpr std::array<direction_t, sizeof...(Directions)> directions{{Directions...}};
	return {directions.data(), directions.data() + directions.size()};
}

// Static direction vectors for `position_t` and `local_position_t`
inline const std::pair<const direction_t*, const direction_t*> local_neighbor_table[] = {
	// Top left corner; xx == 0 && yy == 0
	make_direction_range<
		direction_t::right, direction_t::bottom_right, direction_t::bottom
	>(),
	// Bottom left corner; xx == 0 && yy == 49
	make_direction_range<
		direction_t::top, direction_t::top_right, direction_t::right
	>(),
	// Left side; xx == 0
	make_direction_range<
		direction_t::top, direction_t::top_right,
		direction_t::right,
		direction_t::bottom_right, direction_t::bottom
	>(),
	// Top right corner; xx == 49 && yy == 0
	make_direction_range<
		direction_t::bottom, direction_t::bottom_left, direction_t::left
	>(),
	// Bottom right corner; xx == 49 && yy == 49
	make_direction_range<
		direction_t::top_left, direction_t::top, direction_t::left
	>(),
	// Right side; xx == 49
	make_direction_range<
		direction_t::top_left, direction_t::top,
		direction_t::bottom, direction_t::bottom_left,
		direction_t::left
	>(),
	// Top side; yy == 0
	make_direction_range<
		direction_t::right,
		direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
		direction_t::left
	>(),
	// Bottom side; yy == 49
	make_direction_range<
		direction_t::top_left, direction_t::top, direction_t::top_right,
		direction_t::right, direction_t::left
	>(),
	// Non-border
	make_direction_range<
		direction_t::top_left, direction_t::top, direction_t::top_right,
		direction_t::right,
		direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
		direction_t::left
	>(),
};

// Static direction vectors for `world_position_t`
inline const std::pair<const direction_t*, const direction_t*> world_neighbor_table[] = {
	// Top left corner; xx == 0 && yy == 0
	make_direction_range<direction_t::bottom_right>(),
	// Bottom left corner; xx == 0 && yy == 49
	make_direction_range<direction_t::top_right>(),
	// Left side; xx == 0
	make_direction_range<
		direction_t::top_right, direction_t::right, direction_t::bottom_right,
		direction_t::left
	>(),
	// Top right corner; xx == 49 && yy == 0
	make_direction_range<direction_t::bottom_left>(),
	// Bottom right corner; xx == 49 && yy == 49
	make_direction_range<direction_t::top_left>(),
	// Right side; xx == 49
	make_direction_range<
		direction_t::right,
		direction_t::bottom_left, direction_t::left, direction_t::top_left
	>(),
	// Top side; yy == 0
	make_direction_range<
		direction_t::top,
		direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left
	>(),
	// Bottom side; yy == 49
	make_direction_range<
		direction_t::top_left, direction_t::top, direction_t::top_right,
		direction_t::bottom
	>(),
	// Non-border
	make_direction_range<
		direction_t::top_left, direction_t::top, direction_t::top_right,
		direction_t::right,
		direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
		direction_t::left
	>(),
};

// Used solely by local_position_t::all()
struct all_iteratable_t {
	class iterator : public random_access_iterator_t<iterator> {
		public:
			using value_type = local_position_t;
			using pointer = void;
			using reference = local_position_t;

			constexpr iterator() = default;
			explicit constexpr iterator(int ii) : ii(ii) {}

			constexpr local_position_t operator*() const { return local_position_t(ii % 50, ii / 50); }
			constexpr bool operator==(iterator rhs) const { return ii == rhs.ii; }
			constexpr bool operator<(iterator rhs) const { return ii < rhs.ii; }
			constexpr iterator& operator+=(int val) { ii += val; return *this; }

		private:
			int ii = -1;
	};
	using const_iterator = iterator;

	constexpr iterator begin() const { return iterator(0); }
	constexpr iterator end() const { return iterator(2500); }
};

// Iterates all positions between a top left and bottom right corner position.
template <class Position>
class area_iterable_t {
	public:
		class iterator : public random_access_iterator_t<iterator> {
			public:
				using value_type = Position;
				using pointer = void;
				using reference = Position;

				constexpr iterator() = default;
				constexpr iterator(int offset, int width, int index) : offset(offset), width(width), index(index) {}

				constexpr reference operator*() const { return {offset + index % width, index / width}; }
				constexpr bool operator==(const iterator& rhs) const { return index == rhs.index; }
				constexpr bool operator<(const iterator& rhs) const { return index < rhs.index; }
				constexpr iterator& operator+=(int val) { index += val; return *this; }

			private:
				int offset = 0, width = 0, index = 0;
		};
		using const_iterator = iterator;

		constexpr area_iterable_t(Position top_left, Position bottom_right) :
			offset(top_left.xx),
			width(bottom_right.xx - top_left.xx + 1),
			index(top_left.yy * width),
			final_index(index + std::max(0, (bottom_right.yy - top_left.yy + 1) * width)) {
		}

		constexpr iterator begin() const { return {offset, width, index}; }
		constexpr iterator end() const { return {offset, width, final_index}; }

	private:
		int offset, width, index, final_index;
};

// Iterates all neighbors from an origin.
template <class Position>
class neighbor_iterable_t {
	public:
		class iterator : public random_access_iterator_t<iterator> {
			public:
				using value_type = Position;
				using pointer = void;
				using reference = Position;

				constexpr iterator() = default;
				constexpr iterator(Position origin, const direction_t* direction) : origin(origin), direction(direction) {}

				Position operator*() const { return origin.in_direction(*direction); }
				constexpr bool operator==(const iterator& rhs) const { return direction == rhs.direction; }
				constexpr bool operator<(const iterator& rhs) const { return direction < rhs.direction; }
				constexpr iterator& operator+=(int val) { direction += val; return *this; }

			private:
				Position origin;
				const direction_t* direction = nullptr;
		};
		using const_iterator = iterator;

	public:
		static constexpr neighbor_iterable_t local_neighbors(Position origin) {
			if (origin.xx == 0) {
				if (origin.yy == 0) {
					// Top left corner
					return {origin, local_neighbor_table[0]};
				} else if (origin.yy == 49) {
					// Bottom left corner
					return {origin, local_neighbor_table[1]};
				} else {
					// Left side
					return {origin, local_neighbor_table[2]};
				}
			} else if (origin.xx == 49) {
				if (origin.yy == 0) {
					// Top right corner
					return {origin, local_neighbor_table[3]};
				} else if (origin.yy == 49) {
					// Bottom right corner
					return {origin, local_neighbor_table[4]};
				} else {
					// Right side
					return {origin, local_neighbor_table[5]};
				}
			} else if (origin.yy == 0) {
				// Top side
				return {origin, local_neighbor_table[6]};
			} else if (origin.yy == 49) {
				// Bottom side
				return {origin, local_neighbor_table[7]};
			} else {
				// Non-border
				return {origin, local_neighbor_table[8]};
			}
		}

		static constexpr neighbor_iterable_t world_neighbors(Position origin) {
			if (origin.xx == 0) {
				if (origin.yy == 0) {
					// Top left corner
					return {origin, world_neighbor_table[0]};
				} else if (origin.yy == 49) {
					// Bottom left corner
					return {origin, world_neighbor_table[1]};
				} else {
					// Left side
					return {origin, world_neighbor_table[2]};
				}
			} else if (origin.xx == 49) {
				if (origin.yy == 0) {
					// Top right corner
					return {origin, world_neighbor_table[3]};
				} else if (origin.yy == 49) {
					// Bottom right corner
					return {origin, world_neighbor_table[4]};
				} else {
					// Right side
					return {origin, world_neighbor_table[5]};
				}
			} else if (origin.yy == 0) {
				// Top side
				return {origin, world_neighbor_table[6]};
			} else if (origin.yy == 49) {
				// Bottom side
				return {origin, world_neighbor_table[7]};
			} else {
				// Non-border
				return {origin, world_neighbor_table[8]};
			}
		}

		constexpr iterator begin() const { return {origin, _begin}; }
		constexpr iterator end() const { return {origin, _end}; }

	private:
		constexpr neighbor_iterable_t(
			Position origin, std::pair<const direction_t*, const direction_t*> range
		) : origin(origin), _begin(range.first), _end(range.second) {}

		Position origin;
		const direction_t* _begin;
		const direction_t* _end;
};

// Iterates over all positions that have range equal to a constant. Bounds checking is done once at
// construction and after that iteration is very simple.
template <class Position>
class with_range_iterable_t {
	public:
		class iterator : public random_access_iterator_t<iterator> {
			public:
				using value_type = Position;
				using pointer = void;
				using reference = Position;

				constexpr iterator() = default;
				constexpr iterator(Position origin, int range, int index) : origin(origin), range(range), index(index) {}

				constexpr reference operator*() const {
					return {
						origin.xx + offset(index + range),
						origin.yy + offset(index - range)
					};
				}

				constexpr bool operator==(const iterator& rhs) const { return index == rhs.index; }
				constexpr bool operator<(const iterator& rhs) const { return index < rhs.index; }
				constexpr iterator& operator+=(int val) { index += val; return *this; }

			private:
				Position origin;
				int range = 0, index = 0;

				constexpr int offset(int ii) const {
					return std::clamp(range * 2 - std::abs(ii % (range * 8) - range * 4), -range, range);
				}
		};
		using const_iterator = iterator;

		static constexpr auto local_range(Position position, int range) {
			if (range == 0) {
				// range:0 would break the iterator because you can't modulus by 0. Instead we return a
				// range:1 iterator to the bottom right position and only iterate for one position. Since
				// the iteration starts at the top left this will return an iterator that only outputs the
				// origin.
				if (position.xx == 49 && position.yy == 49) {
					// This avoids creating an invalid position at [50, 50] in case bounds checking is enabled
					return with_range_iterable_t{position.in_direction(direction_t::top_left), 1, 4, 5};
				} else {
					return with_range_iterable_t{position.in_direction(direction_t::bottom_right), 1, 0, 1};
				}
			}

			int begin = 0; // Top-left
			int end = range * 12; // Bottom-right but it goes around 1.5 times
			int bounds = 0;

			// Clamp range horizontally
			if (int dx = range - position.xx; dx > 0) {
				++bounds;
				begin = std::max(begin, dx);
				end = std::min(end, range * 6 - dx + 1);
			}
			if (int dx = 49 - range - position.xx; dx < 0) {
				++bounds;
				begin = std::max(begin, range * 4 - dx);
				end = std::min(end, range * 10 + dx + 1);
			}

			// Clamp range vertically
			if (int dy = range - position.yy; dy > 0) {
				++bounds;
				begin = std::max(begin, range * 2 + dy);
				end = std::min(end, range * 8 - dy + 1);
			}
			if (int dy = 49 - range - position.yy; dy < 0) {
				++bounds;
				if (end >= range * 8) {
					begin = std::max(begin, range * 6 - dy);
					end = std::min(end, range * 12 + dy + 1);
				} else {
					end = std::min(end, range * 4 + dy + 1);
				}
			}

			if (bounds == 4) {
				// Clamped in all directions = no results
				begin = end = 0;
			} else if (end < begin) {
				// Begin wrapped past end
				if (bounds == 3) {
					// In this case it will be a single straight line
					if (position.xx > position.yy) {
						begin = range * 6 + position.yy + range - 49;
					} else {
						begin = range - position.xx;
					}
					end = begin + 50;
				} else {
					std::swap(begin, end);
					end -= begin;
					begin += range * 2;
					end += range * 6;
				}
			} else if (begin + range * 8 < end) {
				// Range is larger than full perimeter
				end -= range * 4;
			}
			return with_range_iterable_t{position, range, begin, end};
		}

		constexpr iterator begin() const { return {origin, range, _begin}; }
		constexpr iterator end() const { return {origin, range, _end}; }

	private:
		constexpr with_range_iterable_t(
			Position origin, int range, int begin, int end
		) : origin(origin), range(range), _begin(begin), _end(end) {
		}

		Position origin;
		int range, _begin, _end;
};

} // namespace detail

//
// direction_t free operators
inline constexpr int operator+(direction_t direction) noexcept {
	return static_cast<int>(direction);
}
inline constexpr direction_t& operator+=(direction_t& direction, int value) noexcept {
	// Double modulus to handle negatives
	direction = static_cast<direction_t>(((+direction + value - 1) % 8 + 8) % 8 + 1);
	return direction;
}
inline constexpr direction_t& operator-=(direction_t& direction, int value) noexcept {
	direction += -value;
	return direction;
}
inline constexpr direction_t& operator++(direction_t& direction) noexcept {
	direction += 1;
	return direction;
}
inline constexpr direction_t& operator--(direction_t& direction) noexcept {
	direction -= 1;
	return direction;
}
inline constexpr direction_t operator++(direction_t& direction, int /* postfix */) noexcept {
	direction_t copy = direction;
	++direction;
	return copy;
}
inline constexpr direction_t operator--(direction_t& direction, int /* postfix */) noexcept {
	direction_t copy = direction;
	--direction;
	return copy;
}
inline constexpr direction_t operator+(direction_t direction, int value) noexcept {
	direction += value;
	return direction;
}
inline constexpr direction_t operator-(direction_t direction, int value) noexcept {
	direction -= value;
	return direction;
}
std::ostream& operator<<(std::ostream& os, direction_t dir);

//
// room_location_t inlines
constexpr room_location_t::room_location_t(std::string_view room_name) : xx(0), yy(0) {
	if (room_name == "sim") {
		xx = yy = 0x80;
	} else {
		const char* vertical_pos = detail::atoi(room_name.begin() + 1, room_name.end(), xx);
		detail::atoi(vertical_pos + 1, room_name.end(), yy);
		if (room_name.front() == 'W') {
			xx = -xx - 1;
		} else if (room_name.front() != 'E') {
			throw std::invalid_argument("Invalid room name");
		}
		if (*vertical_pos == 'N') {
			yy = -yy - 1;
		} else if (*vertical_pos != 'S') {
			throw std::invalid_argument("Invalid room name");
		}
	}
}

constexpr position_t room_location_t::operator[](local_position_t pos) const {
	return {*this, pos};
}

//
// position_t inlines
constexpr position_t::position_t(room_location_t room, local_position_t pos) : xx(pos.xx), yy(pos.yy), room(room) {}
constexpr position_t::position_t(world_position_t pos) : xx(pos.xx % 50), yy(pos.yy % 50), room(pos.room_location()) {}

constexpr local_position_t position_t::operator~() const {
	return {xx, yy};
}

constexpr direction_t position_t::direction_to(position_t that) const {
	return world_position_t(*this).direction_to(that);
}

constexpr position_t position_t::in_direction(direction_t direction) const {
	return world_position_t(*this).in_direction(direction);
}

constexpr bool position_t::near_to(position_t that) const {
	return range_to(that) <= 1;
}

constexpr int position_t::range_to(position_t that) const {
	if (room == that.room) {
		return std::max(detail::abs(xx - that.xx), detail::abs(yy - that.yy));
	} else {
		return INT_MAX;
	}
}

constexpr int position_t::range_to_edge() const {
	return 24 - std::min(
		detail::abs(xx - 24) - xx / 25,
		detail::abs(yy - 24) - yy / 25
	);
}

constexpr auto position_t::neighbors() const {
	return detail::neighbor_iterable_t<position_t>::local_neighbors(*this);
}

//
// local_position_t inlines
constexpr direction_t local_position_t::direction_to(local_position_t that) const {
	return detail::direction_to(*this, that);
}

constexpr local_position_t local_position_t::in_direction(direction_t direction) const {
	return detail::in_direction(*this, direction);
}

constexpr int local_position_t::range_to(local_position_t that) const {
	return std::max(detail::abs(xx - that.xx), detail::abs(yy - that.yy));
}

constexpr int local_position_t::range_to_edge() const {
	return 24 - std::min(
		detail::abs(xx - 24) - xx / 25,
		detail::abs(yy - 24) - yy / 25
	);
}

constexpr auto local_position_t::neighbors() const {
	return detail::neighbor_iterable_t<local_position_t>::local_neighbors(*this);
}

constexpr auto local_position_t::with_range(int range) const {
	return detail::with_range_iterable_t<local_position_t>::local_range(*this, range);
}

constexpr auto local_position_t::within_range(int range) const {
	return detail::area_iterable_t<local_position_t>{
		local_position_t(std::max(0, xx - range), std::max(0, yy - range)),
		local_position_t(std::min(49, xx + range), std::min(49, yy + range))
	};
}

template <class Container>
constexpr auto local_position_t::within_range(int range, Container&& container) const {
	local_position_t top_left(std::max(0, xx - range), std::max(0, yy - range));
	local_position_t bottom_right(std::min(49, xx + range), std::min(49, yy + range));
	for (auto pos : container) {
		top_left = local_position_t(
			std::max<int16_t>(top_left.xx, pos.xx - range),
			std::max<int16_t>(top_left.yy, pos.yy - range)
		);
		bottom_right = local_position_t(
			std::min<int16_t>(bottom_right.xx, pos.xx + range),
			std::min<int16_t>(bottom_right.yy, pos.yy + range)
		);
	}
	return detail::area_iterable_t<local_position_t>{top_left, bottom_right};
}

constexpr auto local_position_t::all() {
	return detail::all_iteratable_t{};
}

constexpr auto local_position_t::area(local_position_t top_left, local_position_t bottom_right) {
	if (bottom_right.xx < top_left.xx || bottom_right.yy < top_left.yy) {
		throw std::domain_error("Invalid area corners");
	}
	return detail::area_iterable_t<local_position_t>{top_left, bottom_right};
}

//
// world_position_t inlines
constexpr world_position_t::world_position_t(position_t position) : world_position_t(
	(position.room.xx + 0x80) * 50u + position.xx,
	(position.room.yy + 0x80) * 50u + position.yy) {
}

constexpr direction_t world_position_t::direction_to(world_position_t that) const {
	return detail::direction_to(*this, that);
}

constexpr world_position_t world_position_t::in_direction(direction_t direction) const {
	return detail::in_direction(*this, direction);
}

constexpr int world_position_t::range_to(world_position_t that) const {
	return std::max(
		xx > that.xx ? xx - that.xx : that.xx - xx,
		yy > that.yy ? yy - that.yy : that.yy - yy
	);
}

constexpr auto world_position_t::neighbors() const {
	return detail::neighbor_iterable_t<world_position_t>::world_neighbors(*this);
}

namespace detail {

// Matrix of any type which holds a value for each position in a room
template <class Type, class Store = void, int Pack = -1>
class local_matrix_store_t;

// Default specialization-- no bitpacking
template <class Type>
class local_matrix_store_t<Type, void, -1> {
	public:
		using reference = Type&;
		using const_reference = const Type&;

		constexpr local_matrix_store_t() = default;
		explicit constexpr local_matrix_store_t(Type value) { fill(value); }

		constexpr reference operator[](int index) { return costs[index]; }
		constexpr const_reference operator[](int index) const { return costs[index]; }

		constexpr Type* data() { return costs.data(); }
		constexpr const Type* data() const { return costs.data(); }
		constexpr void fill(Type value) { std::fill(costs.begin(), costs.end(), value); }

	protected:
		std::array<Type, 2500> costs;
};

// Backing store for local_matrix_t which can bitpack 1, 2, or 4 bits into a matrix
template <class Type, class Store, int Pack>
class local_matrix_store_t {
	private:
		static constexpr int log2(int value) {
			return value == 1 ? 0 : (1 + log2(value >> 1));
		}
		static constexpr int StoreBits = sizeof(Store) << 3;
		static constexpr int IndexShift = log2(StoreBits / Pack);
		static constexpr int IndexMask = StoreBits / Pack - 1;
		static constexpr int IndexBitShift = log2(Pack);
		static constexpr Store Mask = (1 << Pack) - 1;

	public:
		class reference {
			public:
				constexpr reference(int bit_pos, Store& ref) : ref(ref), bit_pos(bit_pos) {}
				constexpr operator Type() const { // NOLINT(hicpp-explicit-conversions)
					return (ref >> bit_pos) & Mask;
				}
				constexpr reference& operator=(Type value) {
					ref = (ref & ~(Mask << bit_pos)) | ((value & Mask) << bit_pos);
					return *this;
				}

			private:
				Store& ref;
				int bit_pos;
		};
		using const_reference = Type;

		constexpr local_matrix_store_t() {
			// Explicitly assign final data entry because otherwise it could be anything and .data() would
			// be inconsistent
			costs.back() = 0;
		}

		explicit constexpr local_matrix_store_t(Type value) {
			costs.back() = 0;
			fill(value);
		}

		constexpr reference operator[](int index) {
			return {(index & IndexMask) << IndexBitShift, costs[index >> IndexShift]};
		}

		constexpr const_reference operator[](int index) const {
			return (costs[index >> IndexShift] >> ((index & IndexMask) << IndexBitShift)) & Mask;
		}

		constexpr Store* data() { return costs.data(); }
		constexpr const Store* data() const { return costs.data(); }

		constexpr void fill(Type value) {
			Store packed{};
			for (int ii = 0; ii < StoreBits; ii += Pack) {
				packed <<= Pack;
				packed |= value & Mask;
			}
			costs.fill(packed);
		}

	protected:
		static_assert(StoreBits >= Pack, "sizeof(Store) must be greater than or equal to pack bits");
		static_assert(StoreBits % Pack == 0, "Store is not aligned to pack bits");
		std::array<Store, (2500 * Pack + StoreBits / Pack) / StoreBits> costs;
};

// Specialization for direction_t packing
template <>
class local_matrix_store_t<direction_t, void, -1> : public local_matrix_store_t<int, int, 4> {
	private:
		using store_t = local_matrix_store_t<int, int, 4>;

	public:
		class reference : store_t::reference {
			public:
				explicit constexpr reference(store_t::reference reference) : store_t::reference(reference) {}
				constexpr reference(int bit_pos, int& ref) : store_t::reference(bit_pos, ref) {}
				constexpr operator direction_t() const { // NOLINT(hicpp-explicit-conversions)
					return static_cast<direction_t>(store_t::reference::operator int() + 1);
				}
				constexpr reference& operator=(direction_t value) {
					store_t::reference::operator=(+value - 1);
					return *this;
				}
		};
		using const_reference = direction_t;

		constexpr local_matrix_store_t() : store_t() {}
		explicit constexpr local_matrix_store_t(direction_t value) : store_t(+value - 1) {}

		constexpr reference operator[](int index) { return reference(store_t::operator[](index)); }
		constexpr const_reference operator[](int index) const { return static_cast<direction_t>(store_t::operator[](index) + 1); }

		constexpr void fill(direction_t value) { store_t::fill(+value - 1); }
};

// Bitpack bools
template <>
class local_matrix_store_t<bool, void, -1> : public local_matrix_store_t<bool, int, 1> {
	public: using local_matrix_store_t<bool, int, 1>::local_matrix_store_t;
};

} // namespace detail

// Extra methods on top of the backing store which accept local_position_t or xx/yy coords
template <class Type, class Store = void, int Pack = -1>
class local_matrix_t : public detail::local_matrix_store_t<Type, Store, Pack> {
	public:
		using reference = typename detail::local_matrix_store_t<Type, Store, Pack>::reference;
		using const_reference = typename detail::local_matrix_store_t<Type, Store, Pack>::const_reference;
		using detail::local_matrix_store_t<Type, Store, Pack>::local_matrix_store_t;
		using detail::local_matrix_store_t<Type, Store, Pack>::operator[];

		template <class Memory>
		void serialize(Memory& memory) {
			auto data = reinterpret_cast<uint8_t*>(this->costs.data());
			memory.copy(data, reinterpret_cast<uint8_t*>(this->costs.data() + this->costs.size()) - data);
		}

		constexpr bool operator==(const local_matrix_t& rhs) const { return this->costs == rhs.costs; }
		constexpr bool operator!=(const local_matrix_t& rhs) const { return !(*this == rhs); }
		constexpr reference operator[](local_position_t pos) { return get(pos.xx, pos.yy); }
		constexpr const_reference operator[](local_position_t pos) const { return get(pos.xx, pos.yy); }

		constexpr reference get(int xx, int yy) { return (*this)[xx * 50 + yy]; }
		constexpr const_reference get(int xx, int yy) const { return (*this)[xx * 50 + yy]; }
		constexpr void set(int xx, int yy, Type cost) { (*this)[xx * 50 + yy] = cost; }
};

} // namespace screeps

// Hash specialization for hashing STL containers
template <> struct std::hash<screeps::room_location_t> {
	constexpr size_t operator()(const screeps::room_location_t& location) const {
		return screeps::detail::flatten(location);
	}
};

template <> struct std::hash<screeps::position_t> {
	constexpr size_t operator()(const screeps::position_t& position) const {
		return screeps::detail::flatten(position);
	}
};

template <> struct std::hash<screeps::local_position_t> {
	constexpr size_t operator()(const screeps::local_position_t& position) const {
		return screeps::detail::flatten(position);
	}
};

template <> struct std::hash<screeps::world_position_t> {
	constexpr size_t operator()(const screeps::world_position_t& position) const {
		return screeps::detail::flatten(position);
	}
};
