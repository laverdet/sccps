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
			constexpr union_t(Type location) : location(location) {}
			Type location;
			uint16_t integer;
		};
		return union_t(location).integer;
	}

	template <class Type>
	constexpr std::enable_if_t<sizeof(Type) == 4, int> flatten(Type location) {
		union union_t {
			constexpr union_t(Type location) : location(location) {}
			Type location;
			int32_t integer;
		};
		return union_t(location).integer;
	}
}

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
	constexpr room_location_t(std::string_view room_name);

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
	constexpr position_t(world_position_t pos);

	template <class Memory> void serialize(Memory& memory) { memory & xx & yy & room; }
	friend std::ostream& operator<<(std::ostream& os, position_t that);

	constexpr bool operator==(position_t rhs) const { return detail::flatten(*this) == detail::flatten(rhs); }
	constexpr bool operator!=(position_t rhs) const { return !(*this == rhs); }
	constexpr bool operator<(position_t rhs) const { return detail::flatten(*this) < detail::flatten(rhs); }
	constexpr bool operator>(position_t rhs) const { return rhs < *this; }
	constexpr bool operator<=(position_t rhs) const { return !(*this > rhs); }
	constexpr bool operator>=(position_t rhs) const { return !(*this < rhs); }
	constexpr local_position_t operator~() const;

	int distance_to(position_t that) const {
		if (room != that.room) {
			return 50;
		}
		return std::max(std::abs(xx - that.xx), std::abs(yy - that.yy));
	}

	constexpr bool near_to(position_t that) const {
		return distance_to(that) <= 1;
	}

	constexpr position_t in_direction(direction_t direction) const;
	constexpr direction_t direction_to(position_t that) const;
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

	int distance_to(local_position_t that) const {
		return std::max(std::abs(xx - that.xx), std::abs(yy - that.yy));
	}

	constexpr local_position_t in_direction(direction_t direction) const {
		switch (direction) {
			case direction_t::top:
				return {xx, yy - 1};
			case direction_t::top_right:
				return {xx + 1, yy - 1};
			case direction_t::right:
				return {xx + 1, yy};
			case direction_t::bottom_right:
				return {xx + 1, yy + 1};
			case direction_t::bottom:
				return {xx, yy + 1};
			case direction_t::bottom_left:
				return {xx - 1, yy + 1};
			case direction_t::left:
				return {xx - 1, yy};
			case direction_t::top_left:
				return {xx - 1, yy - 1};
			default:
				throw std::domain_error("Invalid direction");
		}
	}

	constexpr direction_t direction_to(local_position_t that) const {
		if (xx == that.xx) {
			if (yy == that.yy) {
				throw std::logic_error("`direction_to` same position");
			} else if (yy < that.yy) {
				return direction_t::bottom;
			} else {
				return direction_t::top;
			}
		} else if (yy == that.yy) {
			if (xx < that.xx) {
				return direction_t::right;
			} else {
				return direction_t::left;
			}
		} else if (xx < that.xx) {
			if (yy < that.yy) {
				return direction_t::bottom_right;
			} else {
				return direction_t::top_right;
			}
		} else if (yy < that.yy) {
			return direction_t::bottom_left;
		} else {
			return direction_t::top_left;
		}
	}

	int range_to_edge() const;
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
struct world_position_t {
	uint16_t xx;
	uint16_t yy;

	world_position_t() = default;
	constexpr world_position_t(unsigned xx, unsigned yy) : xx(xx), yy(yy) {}
	constexpr world_position_t(position_t position);

	constexpr world_position_t in_direction(direction_t direction) const;
	constexpr direction_t direction_to(world_position_t that) const;

	constexpr room_location_t room_location() const {
		return {static_cast<int8_t>(xx / 50u - 0x80), static_cast<int8_t>(yy / 50u - 0x80)};
	}

	static const world_position_t null;
};
inline const world_position_t world_position_t::null = world_position_t(UINT_MAX, UINT_MAX);

// TODO: This will be gone
template <class Type, direction_t... Neighbors>
class constexpr_neighbor_iteratable_t {
	public:
		class iterator : public random_access_iterator_t<iterator> {
			private:
				Type origin;
				int index = 0;

			public:
				using value_type = Type;
				using pointer = void;
				using reference = Type;

				constexpr iterator() = default;
				constexpr iterator(Type origin, int index) : origin(origin), index(index) {}

				constexpr reference operator*() const {
					const std::array<direction_t, sizeof...(Neighbors)> directions {{Neighbors...}};
					return origin.in_direction(directions[index]);
				}

				constexpr bool operator==(const iterator& rhs) const {
					return index == rhs.index;
				}

				constexpr bool operator<(const iterator& rhs) const {
					return index > rhs.index;
				}

				constexpr iterator& operator+=(int val) {
					index += val;
					return *this;
				}
		};
		using const_iterator = iterator;

	private:
		Type origin;

	public:
		explicit constexpr constexpr_neighbor_iteratable_t(Type origin) : origin(origin) {}

		constexpr iterator begin() const {
			return {origin, 0};
		}

		constexpr iterator end() const {
			return {origin, sizeof...(Neighbors)};
		}
};

namespace detail {

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
template <class Type>
class area_iterable_t {
	public:
		class iterator : public random_access_iterator_t<iterator> {
			public:
				using value_type = Type;
				using pointer = void;
				using reference = Type;

				constexpr iterator() = default;
				constexpr iterator(int offset, int width, int index) : offset(offset), width(width), index(index) {}

				constexpr reference operator*() const { return {offset + index % width, index / width}; }
				constexpr bool operator==(const iterator& rhs) const { return index == rhs.index; }
				constexpr bool operator<(const iterator& rhs) const { return index > rhs.index; }
				constexpr iterator& operator+=(int val) { index += val; return *this; }

			private:
				int offset = 0, width = 0, index = 0;
		};
		using const_iterator = iterator;

		constexpr area_iterable_t(Type top_left, Type bottom_right) :
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

// Iterates all neighbors from an origin. The directions are calculated once up front and then
// iterated through in order.
template <class Type>
class neighbor_iterable_t {
	public:
		class iterator : public random_access_iterator_t<iterator> {
			public:
				using value_type = Type;
				using pointer = void;
				using reference = Type;

				constexpr iterator() = default;
				constexpr iterator(Type origin, const direction_t* direction) : origin(origin), direction(direction) {}

				Type operator*() const { return origin.in_direction(*direction); }
				constexpr bool operator==(const iterator& rhs) const { return direction == rhs.direction; }
				constexpr bool operator<(const iterator& rhs) const { return direction > rhs.direction; }
				constexpr iterator& operator+=(int val) { direction += val; return *this; }

			private:
				Type origin;
				const direction_t* direction = nullptr;
		};
		using const_iterator = iterator;

	public:
		constexpr neighbor_iterable_t(Type origin, const direction_t* begin, const direction_t* end) : origin(origin), _begin(begin), _end(end) {}

		template <direction_t... Directions>
		static neighbor_iterable_t factory(Type origin) {
			static std::array<direction_t, sizeof...(Directions)> directions{{Directions...}};
			return {origin, directions.data(), directions.data() + directions.size()};
		}

		constexpr iterator begin() const { return {origin, _begin}; }
		constexpr iterator end() const { return {origin, _end}; }

	private:
		Type origin;
		const direction_t* _begin;
		const direction_t* _end;
};

// Iterates over all positions that have range equal to a constant. Bounds checking is done once at
// construction and after that iteration is very simple.
template <class Type>
class with_range_iterable_t {
	public:
		class iterator : public random_access_iterator_t<iterator> {
			public:
				using value_type = Type;
				using pointer = void;
				using reference = Type;

				constexpr iterator() = default;
				constexpr iterator(Type origin, int range, int index) : origin(origin), range(range), index(index) {}

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
				Type origin;
				int range = 0, index = 0;

				constexpr int offset(int ii) const {
					return std::clamp(range * 2 - std::abs(ii % (range * 8) - range * 4), -range, range);
				}
		};
		using const_iterator = iterator;

	private:
		Type origin;
		int range, _begin, _end;

	public:
		constexpr with_range_iterable_t(
			Type _origin, int _range, int begin, int end
		) : origin(_origin), range(_range), _begin(begin), _end(end) {
			if (range == 0) {
				// Fake it for range = 0 because we can't modulus by zero
				++range;
				++origin.xx;
				++origin.yy;
				_begin = 0;
				_end = 1;
			}
		}

		constexpr iterator begin() const { return {origin, range, _begin}; }
		constexpr iterator end() const { return {origin, range, _end}; }
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
inline constexpr direction_t operator++(direction_t direction, int /* postfix */) noexcept {
	// TODO: check this
	return ++direction;
}
inline constexpr direction_t operator--(direction_t direction, int /* postfix */) noexcept {
	return --direction;
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

constexpr position_t position_t::in_direction(direction_t direction) const {
	return world_position_t(*this).in_direction(direction);
}

constexpr direction_t position_t::direction_to(position_t that) const {
	return world_position_t(*this).direction_to(that);
}

using neighbors_iterable = constexpr_neighbor_iteratable_t<position_t,
	direction_t::top_left, direction_t::top, direction_t::top_right,
	direction_t::right,
	direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
	direction_t::left
>;
constexpr auto position_t::neighbors() const {
	return neighbors_iterable{*this};
}

//
// local_position_t inlines
inline int local_position_t::range_to_edge() const {
	return 24 - std::min(
		std::abs(xx - 24) - xx / 25,
		std::abs(yy - 24) - yy / 25
	);
}

constexpr auto local_position_t::neighbors() const {
	if (xx == 0) {
		if (yy == 0) {
			// Top left corner
			return detail::neighbor_iterable_t<local_position_t>::factory<
				direction_t::right, direction_t::bottom_right, direction_t::bottom
			>(*this);
		} else if (yy == 49) {
			// Bottom left corner
			return detail::neighbor_iterable_t<local_position_t>::factory<
				direction_t::top, direction_t::top_right, direction_t::right
			>(*this);
		} else {
			// Left side
			return detail::neighbor_iterable_t<local_position_t>::factory<
				direction_t::top, direction_t::top_right,
				direction_t::right,
				direction_t::bottom_right, direction_t::bottom
			>(*this);
		}
	} else if (xx == 49) {
		if (yy == 0) {
			// Top right corner
			return detail::neighbor_iterable_t<local_position_t>::factory<
				direction_t::bottom, direction_t::bottom_left, direction_t::left
			>(*this);
		} else if (yy == 49) {
			// Bottom right corner
			return detail::neighbor_iterable_t<local_position_t>::factory<
				direction_t::top_left, direction_t::top, direction_t::left
			>(*this);
		} else {
			// Right side
			return detail::neighbor_iterable_t<local_position_t>::factory<
				direction_t::top_left, direction_t::top,
				direction_t::bottom, direction_t::bottom_left,
				direction_t::left
			>(*this);
		}
	} else if (yy == 0) {
		// Top side
		return detail::neighbor_iterable_t<local_position_t>::factory<
			direction_t::right,
			direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
			direction_t::left
		>(*this);
	} else if (yy == 49) {
		// Bottom side
		return detail::neighbor_iterable_t<local_position_t>::factory<
			direction_t::top_left, direction_t::top, direction_t::top_right,
			direction_t::right, direction_t::left
		>(*this);
	} else {
		// Non-border
		return detail::neighbor_iterable_t<local_position_t>::factory<
			direction_t::top_left, direction_t::top, direction_t::top_right,
			direction_t::right,
			direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
			direction_t::left
		>(*this);
	}
}

constexpr auto local_position_t::with_range(int range) const {
	int begin = 0; // Top-left
	int end = range * 12; // Bottom-right but it goes around 1.5 times
	int bounds = 0;

	// Clamp range horizontally
	if (int dx = range - xx; dx > 0) {
		++bounds;
		begin = std::max(begin, dx);
		end = std::min(end, range * 6 - dx + 1);
	}
	if (int dx = 49 - range - xx; dx < 0) {
		++bounds;
		begin = std::max(begin, range * 4 - dx);
		end = std::min(end, range * 10 + dx + 1);
	}

	// Clamp range vertically
	if (int dy = range - yy; dy > 0) {
		++bounds;
		begin = std::max(begin, range * 2 + dy);
		end = std::min(end, range * 8 - dy + 1);
	}
	if (int dy = 49 - range - yy; dy < 0) {
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
			if (xx > yy) {
				begin = range * 6 + yy + range - 49;
			} else {
				begin = range - xx;
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
	return detail::with_range_iterable_t<local_position_t>{*this, range, begin, end};
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

constexpr world_position_t world_position_t::in_direction(direction_t direction) const {
	switch (direction) {
		case direction_t::top:
			return {xx, yy - 1u};
		case direction_t::top_right:
			return {xx + 1u, yy - 1u};
		case direction_t::right:
			return {xx + 1u, yy};
		case direction_t::bottom_right:
			return {xx + 1u, yy + 1u};
		case direction_t::bottom:
			return {xx, yy + 1u};
		case direction_t::bottom_left:
			return {xx - 1u, yy + 1u};
		case direction_t::left:
			return {xx - 1u, yy};
		case direction_t::top_left:
			return {xx - 1u, yy - 1u};
		default:
			throw std::domain_error("Invalid direction");
	}
}

constexpr direction_t world_position_t::direction_to(world_position_t that) const {
	if (xx == that.xx) {
		if (yy == that.yy) {
			throw std::logic_error("`direction_to` same position");
		} else if (yy < that.yy) {
			return direction_t::bottom;
		} else {
			return direction_t::top;
		}
	} else if (yy == that.yy) {
		if (xx < that.xx) {
			return direction_t::right;
		} else {
			return direction_t::left;
		}
	} else if (xx < that.xx) {
		if (yy < that.yy) {
			return direction_t::bottom_right;
		} else {
			return direction_t::top_right;
		}
	} else if (yy < that.yy) {
		return direction_t::bottom_left;
	} else {
		return direction_t::top_left;
	}
}

// Matrix of any type which holds a value for each position in a room
template <class Type, class Store, size_t Pack, bool Packed = sizeof(Store) << 3 != Pack>
class local_matrix_store_t {};

template <class Type, class Store, size_t Pack>
class local_matrix_store_t<Type, Store, Pack, false> {
	protected:
		std::array<Store, 2500> costs;
		using reference = Type&;
		using const_reference = const Type&;

	public:
		constexpr local_matrix_store_t() = default;
		explicit constexpr local_matrix_store_t(Type value) {
			fill(value);
		}

		constexpr void fill(Type value) {
			std::fill(costs.begin(), costs.end(), value);
		}

		constexpr reference operator[](int index) {
			return costs[index];
		}

		constexpr const_reference operator[](int index) const {
			return costs[index];
		}

		constexpr Store* data() {
			return costs.data();
		}

		constexpr const Store* data() const {
			return costs.data();
		}
};

// Backing store for local_matrix_t which can bitpack 1, 2, or 4 bits into a matrix
template <class Type, class Store, size_t Pack>
class local_matrix_store_t<Type, Store, Pack, true> {
	protected:
		static constexpr int log2(size_t value) {
			return value == 1 ? 0 : (1 + log2(value >> 1));
		}
		static constexpr int StoreBits = sizeof(Store) << 3;
		static constexpr int IndexShift = log2(StoreBits / Pack);
		static constexpr int IndexMask = StoreBits / Pack - 1;
		static constexpr int IndexBitShift = log2(Pack);
		static constexpr Store Mask = (1 << Pack) - 1;

		static_assert(StoreBits >= Pack, "sizeof(Store) must be greater than or equal to pack bits");
		static_assert(StoreBits % Pack == 0, "Store is not aligned to pack bits");

		std::array<Store, (2500 * Pack + StoreBits / Pack) / StoreBits> costs;

		class reference {
			private:
				Store& ref;
				int bit_pos;

			public:
				constexpr reference(int bit_pos, Store& ref) : ref(ref), bit_pos(bit_pos) {}
				constexpr operator Type() const { // NOLINT(hicpp-explicit-conversions)
					return (ref >> bit_pos) & Mask;
				}
				constexpr reference& operator=(Type value) {
					ref = (ref & ~(Mask << bit_pos)) | ((value & Mask) << bit_pos);
					return *this;
				}
		};
		using const_reference = Type;

	public:
		constexpr local_matrix_store_t() {
			// Explicitly assign final data entry because otherwise it could be anything and .data() would
			// be inconsistent
			costs.back() = 0;
		}

		explicit constexpr local_matrix_store_t(Type value) {
			costs.back() = 0;
			fill(value);
		}

		constexpr void fill(Type value) {
			Store packed{};
			for (int ii = 0; ii < StoreBits; ii += Pack) {
				packed <<= Pack;
				packed |= value & Mask;
			}
			costs.fill(packed);
		}

		constexpr reference operator[](int index) {
			return {(index & IndexMask) << IndexBitShift, costs[index >> IndexShift]};
		}

		constexpr const_reference operator[](int index) const {
			return (costs[index >> IndexShift] >> ((index & IndexMask) << IndexBitShift)) & Mask;
		}

		constexpr Store* data() {
			return costs.data();
		}

		constexpr const Store* data() const {
			return costs.data();
		}
};

// Extra methods on top of the backing store which accept local_position_t or xx/yy coords
template <
	class Type,
	class Store = std::conditional_t<std::is_same_v<Type, bool>, uint32_t, Type>,
	size_t Pack = std::conditional_t<
		std::is_same<Type, bool>::value,
		std::integral_constant<size_t, 1>,
		std::integral_constant<size_t, sizeof(Store) << 3>
	>::value
>
class local_matrix_t : public local_matrix_store_t<Type, Store, Pack> {
	public:
		using local_matrix_store_t<Type, Store, Pack>::local_matrix_store_t;
		using local_matrix_store_t<Type, Store, Pack>::operator[];
		using reference = typename local_matrix_store_t<Type, Store, Pack>::reference;
		using const_reference = typename local_matrix_store_t<Type, Store, Pack>::const_reference;

		template <class Memory>
		void serialize(Memory& memory) {
			auto data = reinterpret_cast<uint8_t*>(this->costs.data());
			memory.copy(data, reinterpret_cast<uint8_t*>(this->costs.data() + this->costs.size()) - data);
		}

		constexpr bool operator==(const local_matrix_t& rhs) const {
			return this->costs == rhs.costs;
		}

		constexpr bool operator!=(const local_matrix_t& rhs) const {
			return this->costs != rhs.costs;
		}

		constexpr reference get(int xx, int yy) {
			return (*this)[xx * 50 + yy];
		}

		constexpr const_reference get(int xx, int yy) const {
			return (*this)[xx * 50 + yy];
		}

		constexpr void set(int xx, int yy, Type cost) {
			(*this)[xx * 50 + yy] = cost;
		}

		constexpr reference operator[](local_position_t pos) {
			return get(pos.xx, pos.yy);
		}

		constexpr const_reference operator[](local_position_t pos) const {
			return get(pos.xx, pos.yy);
		}
};

} // namespace screeps

// Hash specialization for hashing STL containers
template <> struct std::hash<screeps::room_location_t> {
	size_t operator()(const screeps::room_location_t& location) const {
		return screeps::detail::flatten(location);
	}
};

template <> struct std::hash<screeps::position_t> {
	size_t operator()(const screeps::position_t& position) const {
		return screeps::detail::flatten(position);
	}
};

template <> struct std::hash<screeps::local_position_t> {
	size_t operator()(const screeps::local_position_t& position) const {
		return screeps::detail::flatten(position);
	}
};

template <> struct std::hash<screeps::world_position_t> {
	size_t operator()(const screeps::world_position_t& position) const {
		return screeps::detail::flatten(position);
	}
};
