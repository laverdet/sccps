#pragma once
#include "./iterator.h"
#include <array>
#include <iosfwd>
#include <limits>
#include <unordered_map>

namespace screeps {

// These will match the constants in game
enum struct direction_t { top = 1, top_right, right, bottom_right, bottom, bottom_left, left, top_left };
inline constexpr int operator+(direction_t resource) noexcept {
	return static_cast<int>(resource);
}
std::ostream& operator<<(std::ostream& os, direction_t dir);

// Abstract neighbor iterators
template <typename Type, direction_t... Neighbors>
class constexpr_neighbor_iteratable_t {
	private:
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

template <typename Type>
class dynamic_neighbor_iterable_t {
	private:
		class iterator : public random_access_iterator_t<iterator> {
			private:
				Type origin;
				const direction_t* direction = nullptr;

			public:
				using value_type = Type;
				using pointer = void;
				using reference = Type;

				constexpr iterator() = default;
				constexpr iterator(Type origin, const direction_t* direction) : origin(origin), direction(direction) {}

				Type operator*() const {
					return origin.in_direction(*direction);
				}

				constexpr bool operator==(const iterator& rhs) const {
					return direction == rhs.direction;
				}

				constexpr bool operator<(const iterator& rhs) const {
					return direction > rhs.direction;
				}

				constexpr iterator& operator+=(int val) {
					direction += val;
					return *this;
				}
		};
		Type origin;
		const direction_t* _begin;
		const direction_t* _end;

	public:
		constexpr dynamic_neighbor_iterable_t(Type origin, const direction_t* begin, const direction_t* end) : origin(origin), _begin(begin), _end(end) {}

		template <direction_t... Directions>
		static dynamic_neighbor_iterable_t factory(Type origin) {
			static std::array<direction_t, sizeof...(Directions)> directions{{Directions...}};
			return {origin, directions.data(), directions.data() + directions.size()};
		}

		constexpr iterator begin() const {
			return {origin, _begin};
		}

		constexpr iterator end() const {
			return {origin, _end};
		}
};

template <typename Type>
class area_iterable_t {
	private:
		class iterator : public random_access_iterator_t<iterator> {
			private:
				unsigned int offset = 0, width = 0, index = 0;

			public:
				using value_type = Type;
				using pointer = void;
				using reference = Type;

				constexpr iterator() = default;
				constexpr iterator(unsigned int offset, unsigned int width, unsigned int index) : offset(offset), width(width), index(index) {}

				constexpr reference operator*() const {
					return {offset + index % width, index / width, 0};
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

		unsigned int offset, width, index, final_index;

	public:
		constexpr area_iterable_t(Type top_left, Type bottom_right) :
			offset(top_left.ux),
			width(bottom_right.ux - top_left.ux + 1),
			index(top_left.uy * width),
			final_index(index + std::max(0u, (bottom_right.uy - top_left.uy + 1) * width)) {
		}

		constexpr iterator begin() const {
			return {offset, width, index};
		}

		constexpr iterator end() const {
			return {offset, width, final_index};
		}
};

template <typename Type>
class with_range_iterable_t {
	private:
		class iterator : public random_access_iterator_t<iterator> {
			private:
				Type origin;
				int range = 0, index = 0;

				constexpr int offset(int ii) const {
					return std::clamp(range * 2 - std::abs(ii % (range * 8) - range * 4), -range, range);
				}

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

				constexpr bool operator==(const iterator& rhs) const {
					return index == rhs.index;
				}

				constexpr bool operator<(const iterator& rhs) const {
					return index < rhs.index;
				}

				constexpr iterator& operator+=(int val) {
					index += val;
					return *this;
				}
		};

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

		constexpr iterator begin() const {
			return {origin, range, _begin};
		}

		constexpr iterator end() const {
			return {origin, range, _end};
		}
};

// Base class for xx/yy pairs which implements some commonalities between the various game positions
template <typename Derived>
struct coord_base_t {
	union {
		uint32_t id;
		struct {
			int16_t xx, yy;
		};
		struct {
			uint16_t ux, uy;
		};
	};

	coord_base_t() = default; // NOLINT(hicpp-member-init)
	explicit constexpr coord_base_t(uint32_t id) : id(id) {}
	constexpr coord_base_t(int xx, int yy) : xx(xx), yy(yy) {}
	constexpr coord_base_t(unsigned int ux, unsigned int uy, int /* unsigned */) : ux(ux), uy(uy) {}

	template <class Memory>
	void serialize(Memory& memory) {
		memory & id;
	}

	constexpr bool operator==(coord_base_t rhs) const {
		return id == rhs.id;
	}

	constexpr bool operator!=(coord_base_t rhs) const {
		return id != rhs.id;
	}

	constexpr bool operator<(coord_base_t rhs) const {
		return id < rhs.id;
	}

	constexpr bool operator>(coord_base_t rhs) const {
		return id > rhs.id;
	}

	constexpr int distance_to(coord_base_t that) const {
		return std::max(std::abs(xx - that.xx), std::abs(yy - that.yy));
	}

	constexpr bool near_to(coord_base_t that) const {
		return distance_to(that) <= 1;
	}

	constexpr Derived in_direction(direction_t direction) const {
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

	constexpr direction_t direction_to(coord_base_t that) const {
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

	static constexpr area_iterable_t<Derived> area(Derived top_left, Derived bottom_right) {
		if (bottom_right.xx < top_left.xx || bottom_right.yy < top_left.yy) {
			throw std::domain_error("Invalid area corners");
		}
		return {top_left, bottom_right};
	}
};

// This is replacement for `roomName` in the JS API.
// "E0S0" -> { xx: 128, yy: 128 }
struct room_location_t : coord_base_t<room_location_t> {
	using coord_base_t::coord_base_t;

	constexpr struct position_t operator[](struct local_position_t pos) const;

	const class terrain_t& terrain(class terrain_t* terrain = nullptr) const;
	friend std::ostream& operator<<(std::ostream& os, room_location_t that);
};

// Simple container for a location in an arbitrary room
struct local_position_t : coord_base_t<local_position_t> {
	using coord_base_t::coord_base_t;

	friend std::ostream& operator<<(std::ostream& os, local_position_t that);
	constexpr struct position_t in_room(room_location_t room) const;

	int range_to_edge() const {
		return 24 - std::min(
			std::abs(xx - 24) - xx / 25,
			std::abs(yy - 24) - yy / 25
		);
	}

	constexpr dynamic_neighbor_iterable_t<local_position_t> neighbors() const {
		if (xx == 0) {
			if (yy == 0) {
				// Top left corner
				return dynamic_neighbor_iterable_t<local_position_t>::factory<
					direction_t::right, direction_t::bottom_right, direction_t::bottom
				>(*this);
			} else if (yy == 49) {
				// Bottom left corner
				return dynamic_neighbor_iterable_t<local_position_t>::factory<
					direction_t::top, direction_t::top_right, direction_t::right
				>(*this);
			} else {
				// Left side
				return dynamic_neighbor_iterable_t<local_position_t>::factory<
					direction_t::top, direction_t::top_right,
					direction_t::right,
					direction_t::bottom_right, direction_t::bottom
				>(*this);
			}
		} else if (xx == 49) {
			if (yy == 0) {
				// Top right corner
				return dynamic_neighbor_iterable_t<local_position_t>::factory<
					direction_t::bottom, direction_t::bottom_left, direction_t::left
				>(*this);
			} else if (yy == 49) {
				// Bottom right corner
				return dynamic_neighbor_iterable_t<local_position_t>::factory<
					direction_t::top_left, direction_t::top, direction_t::left
				>(*this);
			} else {
				// Right side
				return dynamic_neighbor_iterable_t<local_position_t>::factory<
					direction_t::top_left, direction_t::top,
					direction_t::bottom, direction_t::bottom_left,
					direction_t::left
				>(*this);
			}
		} else if (yy == 0) {
			// Top side
			return dynamic_neighbor_iterable_t<local_position_t>::factory<
				direction_t::right,
				direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
				direction_t::left
			>(*this);
		} else if (yy == 49) {
			// Bottom side
			return dynamic_neighbor_iterable_t<local_position_t>::factory<
				direction_t::top_left, direction_t::top, direction_t::top_right,
				direction_t::right, direction_t::left
			>(*this);
		} else {
			// Non-border
			return dynamic_neighbor_iterable_t<local_position_t>::factory<
				direction_t::top_left, direction_t::top, direction_t::top_right,
				direction_t::right,
				direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
				direction_t::left
			>(*this);
		}
	}

	constexpr with_range_iterable_t<local_position_t> with_range(int range) const {
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
		return {*this, range, begin, end};
	}

	constexpr area_iterable_t<local_position_t> within_range(int range) const {
		return {
			local_position_t(std::max(0, xx - range), std::max(0, yy - range)),
			local_position_t(std::min(49, xx + range), std::min(49, yy + range))
		};
	}

	template <class Container>
	constexpr area_iterable_t<local_position_t> within_range(int range, Container&& container) const {
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
		return {top_left, bottom_right};
	}

	struct all_iteratable_t {
		struct iterator : public random_access_iterator_t<iterator> {
			private:
				int ii = 0;

			public:
				using value_type = local_position_t;
				using pointer = void;
				using reference = local_position_t;

				constexpr iterator() = default;
				explicit constexpr iterator(int ii) : ii(ii) {}

				constexpr local_position_t operator*() const {
					return local_position_t(ii % 50, ii / 50);
				}

				constexpr bool operator==(const iterator& rhs) const {
					return ii == rhs.ii;
				}

				constexpr bool operator<(const iterator& rhs) const {
					return ii < rhs.ii;
				}

				constexpr iterator& operator+=(int val) {
					ii += val;
					return *this;
				}
		};

		constexpr iterator begin() const {
			return iterator(0);
		}

		constexpr iterator end() const {
			return iterator(2500);
		}
	};

	static constexpr all_iteratable_t all() {
		return {};
	}
};

// This represents a position in a continuous plane of the whole world. `roomName` is implied from
// xx and yy.
struct position_t : coord_base_t<position_t> {
	using coord_base = coord_base_t<position_t>;
	using coord_base::coord_base_t;
	constexpr position_t(room_location_t room, local_position_t pos) : coord_base(room.ux * 50 + pos.ux, room.uy * 50 + pos.uy, 0) {}
	constexpr position_t(room_location_t room, int xx, int yy) : coord_base(room.ux * 50 + xx, room.uy * 50 + yy, 0) {}

	constexpr room_location_t room_location() const {
		return {ux / 50u, uy / 50u, 0};
	}

	constexpr local_position_t to_local() const {
		return ~*this;
	}

	constexpr local_position_t operator~() const {
		return {ux % 50u, uy % 50u, 0};
	}

	friend std::ostream& operator<<(std::ostream& os, position_t that);

	using neighbors_iterable = constexpr_neighbor_iteratable_t<position_t,
		direction_t::top_left, direction_t::top, direction_t::top_right,
		direction_t::right,
		direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
		direction_t::left
	>;
	constexpr neighbors_iterable neighbors() const {
		return neighbors_iterable{*this};
	}
};

// Definitions for methods with cross-dependencies
constexpr position_t local_position_t::in_room(room_location_t room) const {
	return {room, *this};
}

constexpr position_t room_location_t::operator[](local_position_t pos) const {
	return {*this, pos};
}

// Matrix of any type which holds a value for each position in a room
template <typename Type, typename Store, size_t Pack, bool Packed = sizeof(Store) << 3 != Pack>
class local_matrix_store_t {};

template <typename Type, typename Store, size_t Pack>
class local_matrix_store_t<Type, Store, Pack, false> {
	protected:
		std::array<Store, 2500> costs;
		using reference = Type&;
		using const_reference = typename std::conditional<std::is_trivial<Type>::value && sizeof(Type) <= sizeof(size_t), Type, const Type&>::type;

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
template <typename Type, typename Store, size_t Pack>
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
	typename Type,
	typename Store = typename std::conditional<std::is_same<Type, bool>::value, uint32_t, Type>::type,
	size_t Pack = std::conditional<
		std::is_same<Type, bool>::value,
		std::integral_constant<size_t, 1>,
		std::integral_constant<size_t, sizeof(Store) << 3>
	>::type::value
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

template <class First, class Second>
using first_t = First;

// Hash specialization for hashing STL containers
template <> struct std::hash<screeps::room_location_t> {
	auto operator()(const screeps::room_location_t& val) const {
		return std::hash<decltype(val.id)>()(val.id);
	}
};

template <> struct std::hash<screeps::local_position_t> {
	auto operator()(const screeps::local_position_t& val) const {
		return std::hash<decltype(val.id)>()(val.id);
	}
};

template <> struct std::hash<screeps::position_t> {
	auto operator()(const screeps::position_t& val) const {
		return std::hash<decltype(val.id)>()(val.id);
	}
};
