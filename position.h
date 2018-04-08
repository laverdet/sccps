#pragma once
#include "./iterator.h"
#include "./memory.h"
#include <array>
#include <iostream>
#include <limits>
#include <unordered_map>

namespace screeps {

// These will match the constants in game
enum struct direction_t { top = 1, top_right, right, bottom_right, bottom, bottom_left, left, top_left };

// Abstract neighbor iterators
template <typename Type, direction_t... Neighbors>
class constexpr_neighbor_iteratable_t {
	private:
		class iterator : public random_access_iterator_t<iterator> {
			private:
				Type origin;
				uint8_t index = 0;

			public:
				using value_type = Type;
				using pointer = void;
				using reference = Type;

				iterator() = default;
				constexpr iterator(Type origin) : origin(origin), index(sizeof...(Neighbors)) {}
				constexpr iterator(Type origin, uint8_t index) : origin(origin), index(index) {}

				constexpr reference operator*() const {
					const std::array<direction_t, sizeof...(Neighbors)> directions {{Neighbors...}};
					return origin.in_direction(directions[index - 1]);
				}

				constexpr bool operator==(const iterator& rhs) const {
					return index == rhs.index;
				}

				constexpr bool operator<(const iterator& rhs) const {
					return index > rhs.index;
				}

				constexpr iterator& operator+=(int val) {
					index -= val;
					return *this;
				}

				constexpr iterator operator+(int val) const {
					return iterator(origin, index - val);
				}
		};
		Type origin;

	public:
		constexpr constexpr_neighbor_iteratable_t(Type origin) : origin(origin) {}

		constexpr iterator begin() const {
			return iterator(origin);
		}

		constexpr iterator end() const {
			return iterator();
		}
};

template <typename Type>
class dynamic_neighbor_iterable_t {
	private:
		class iterator : public forward_iterator_t<iterator> {
			private:
				Type origin;
				const direction_t* directions;
				uint8_t index = 0;

			public:
				using value_type = Type;
				using pointer = void;
				using reference = Type;

				iterator() = default;
				constexpr iterator(Type origin, const direction_t* directions, uint8_t index) : origin(origin), directions(directions), index(index) {}

				Type operator*() const {
					return origin.in_direction(directions[index - 1]);
				}

				constexpr bool operator==(const iterator& rhs) const {
					return index == rhs.index;
				}

				constexpr iterator& operator++() {
					--index;
					return *this;
				}
		};
		Type origin;
		const direction_t* directions;
		uint8_t size;

	public:
		constexpr dynamic_neighbor_iterable_t(Type origin, const direction_t* directions, uint8_t size) : origin(origin), directions(directions), size(size) {}

		template <direction_t... Directions>
		static auto reverse_directions() {
			std::array<direction_t, sizeof...(Directions)> tmp{{Directions...}};
			std::reverse(tmp.begin(), tmp.end());
			return tmp;
		}

		template <direction_t... Directions>
		static dynamic_neighbor_iterable_t factory(Type origin) {
			static std::array<direction_t, sizeof...(Directions)> directions(reverse_directions<Directions...>());
			return dynamic_neighbor_iterable_t(origin, directions.data(), directions.size());
		}

		constexpr iterator begin() const {
			return iterator(origin, directions, size);
		}

		constexpr iterator end() const {
			return iterator();
		}
};

// Base class for xx/yy pairs which implements some commonalities between the various game positions
template <typename Derived, typename Integral, typename IntegralUnion>
struct coord_base_t {
	union {
		IntegralUnion id;
		struct {
			Integral xx, yy;
		};
	};
	static constexpr size_t max = std::numeric_limits<IntegralUnion>::max();

	coord_base_t() = default;
	coord_base_t(memory_t& memory) {
		memory <<*this;
	}
	constexpr coord_base_t(Integral xx, Integral yy) : xx(xx), yy(yy) {}
	constexpr coord_base_t(const coord_base_t& that) = default;

	friend memory_t& operator<<(memory_t& memory, coord_base_t that) {
		memory <<that.xx <<that.yy;
		return memory;
	}

	friend memory_t& operator>>(memory_t& memory, coord_base_t& that) {
		memory >>that.xx >>that.yy;
		return memory;
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

	uint8_t distance_to(coord_base_t that) const {
		return std::max(std::abs(xx - that.xx), std::abs(yy - that.yy));
	}

	bool near_to(coord_base_t that) const {
		return distance_to(that) <= 1;
	}

	constexpr Derived in_direction(direction_t direction) const {
		switch (direction) {
			case direction_t::top:
				return Derived(xx, yy - 1);
			case direction_t::top_right:
				return Derived(xx + 1, yy - 1);
			case direction_t::right:
				return Derived(xx + 1, yy);
			case direction_t::bottom_right:
				return Derived(xx + 1, yy + 1);
			case direction_t::bottom:
				return Derived(xx, yy + 1);
			case direction_t::bottom_left:
				return Derived(xx - 1, yy + 1);
			case direction_t::left:
				return Derived(xx - 1, yy);
			case direction_t::top_left:
				return Derived(xx - 1, yy - 1);
			default:
				throw std::logic_error("Invalid direction");
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
};

// This is replacement for `roomName` in the JS API.
// "E0S0" -> { xx: 128, yy: 128 }
struct room_location_t : coord_base_t<room_location_t, uint8_t, uint16_t> {
	using coord_base_t<room_location_t, uint8_t, uint16_t>::coord_base_t;
	room_location_t() = default;
	constexpr room_location_t(const room_location_t&) = default;

	const class terrain_t& terrain() const;
	friend std::ostream& operator<<(std::ostream& os, room_location_t that);

	struct circle_t {
		double radius = 0.15;
		uint32_t fill = 0xffffff;
		double opacity = 0.5;
		int32_t stroke = -1;
		double stroke_width = 0.1;
		// line_style
	};
	void draw_circle(double xx, double yy, const circle_t& options) const;
	struct text_t {
		uint32_t color = 0xffffff;
		std::string font = "";
		uint32_t stroke = -1;
		double stroke_width = 0.15;
		int32_t background_color = -1;
		double background_padding = 0.3;
		std::string align = "center";
		double opacity = 1;
	};
	void draw_text(double xx, double yy, const std::string& text, const text_t& options) const;
};

// Simple container for a location in an arbitrary room
struct local_position_t : coord_base_t<local_position_t, int8_t, uint16_t> {
	using coord_base_t<local_position_t, int8_t, uint16_t>::coord_base_t;
	local_position_t() = default;
	constexpr local_position_t(const local_position_t&) = default;

	friend std::ostream& operator<<(std::ostream& os, local_position_t that);
	constexpr struct position_t in_room(room_location_t room) const;

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

	struct with_range_iterable_t {
		class iterator : public forward_iterator_t<iterator> {
			private:
				uint8_t range2;
				int8_t xx, yy;
				int8_t dx = 1;
				int8_t dy = 0;
				uint16_t tick = 0;

			public:
				using value_type = local_position_t;
				using pointer = void;
				using reference = local_position_t;

				iterator() = default;
				constexpr iterator(uint16_t tick) : range2(0), xx(0), yy(0), tick(tick) {}
				constexpr iterator(int8_t xx, int8_t yy, uint8_t range) : range2(range * 2), xx(xx - range), yy(yy - range) {
					if (this->xx < 0 || this->yy < 0) {
						++(*this);
					}
				}

				constexpr local_position_t operator*() const {
					return local_position_t(xx, yy);
				}

				constexpr bool operator==(const iterator& rhs) const {
					return tick == rhs.tick;
				}

				constexpr iterator& operator++() {
					xx += dx;
					yy += dy;
					++tick;
					if (tick % range2 == 0) {
						switch (tick / range2) {
							case 1:
								dx = 0;
								dy = 1;
								break;
							case 2:
								dx = -1;
								dy = 0;
								break;
							case 3:
								dx = 0;
								dy = -1;
								break;
						}
					}
					if ((xx < 0 || xx > 49 || yy < 0 || yy > 49) && tick < range2 * 4) {
						++*this;
					}
					return *this;
				}
		};
		int8_t xx, yy, range;
		constexpr with_range_iterable_t(local_position_t origin, uint8_t range) : xx(origin.xx), yy(origin.yy), range(range) {}
		constexpr iterator begin() const {
			return iterator(xx, yy, range);
		}
		constexpr iterator end() const {
			return iterator(range * 8);
		}
	};
	with_range_iterable_t with_range(uint8_t range) const {
		return with_range_iterable_t(*this, range);
	}

	struct within_range_iterable_t {
		class iterator : public forward_iterator_t<iterator> {
			private:
				uint8_t xx, yy, range;
				with_range_iterable_t::iterator it;
				with_range_iterable_t::iterator it_end;

			public:
				using value_type = local_position_t;
				using pointer = void;
				using reference = local_position_t;

				iterator() = default;
				constexpr iterator(uint8_t xx, uint8_t yy) : xx(xx), yy(yy), range(1), it(xx, yy, 1), it_end(8) {}
				constexpr iterator(uint8_t range) : xx(0), yy(0), range(range), it(0), it_end(0) {}

				constexpr local_position_t operator*() const {
					return *it;
				}

				constexpr bool operator==(const iterator& rhs) const {
					return range == rhs.range;
				}

				constexpr iterator& operator++() {
					++it;
					if (!(it != it_end)) {
						it = with_range_iterable_t::iterator(xx, yy, ++range);
						it_end = with_range_iterable_t::iterator(range * 8);
					}
					return *this;
				}
		};
		uint8_t xx, yy, range;
		constexpr within_range_iterable_t(uint8_t xx, uint8_t yy, uint8_t range) : xx(xx), yy(yy), range(range) {}
		constexpr iterator begin() const {
			return iterator(xx, yy);
		}
		constexpr iterator end() const {
			return iterator(range + 1);
		}
	};
	within_range_iterable_t within_range(uint8_t range) const {
		return within_range_iterable_t(xx, yy, range);
	}

	/*
	struct area_iterable_t {
		struct iterator {
			uint8_t xx, yy;
			uint8_t origin_x, last_x;
			constexpr iterator(uint8_t xx, uint8_t yy, uint8_t last_x) : xx(xx), yy(yy), origin_x(xx), last_x(last_x) {}
			constexpr iterator(uint8_t xx, uint8_t yy) : xx(xx), yy(yy), origin_x(0), last_x(0) {}
			constexpr bool operator!=(const iterator rhs) const {
				return xx != rhs.xx || yy != rhs.yy;
			}
			constexpr iterator& operator++() {
				if (xx == last_x) {
					xx = origin_x;
					++yy;
				} else {
					++xx;
				}
			}
		};
		uint8_t x1, y1, x2, y2;
		constexpr area_iterable_t(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}
		constexpr iterator begin() const {
			return iterator(x1, y1, x2);
		}
		constexpr iterator end() const {
			return iterator(x1, y2 + 1);
		}
	};
	*/

	struct all_iteratable_t {
		struct iterator : public forward_iterator_t<iterator> {
			private:
				uint16_t ii;

			public:
				using value_type = local_position_t;
				using pointer = void;
				using reference = local_position_t;

				iterator() = default;
				constexpr iterator(uint16_t ii) : ii(ii) {}

				constexpr local_position_t operator*() const {
					return local_position_t(ii % 50, ii / 50);
				}

				constexpr bool operator==(const iterator& rhs) const {
					return ii == rhs.ii;
				}

				constexpr iterator& operator++() {
					++ii;
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
		return all_iteratable_t();
	}
};

// This represents a position in a continuous plane of the whole world. `roomName` is implied from
// xx and yy.
struct position_t : coord_base_t<position_t, uint16_t, uint32_t> {
	using coord_base = coord_base_t<position_t, uint16_t, uint32_t>;
	using coord_base::coord_base_t;
	position_t() = default;
	constexpr position_t(const position_t&) = default;
	constexpr position_t(room_location_t room, local_position_t pos) : coord_base(room.xx * 50 + pos.xx, room.yy * 50 + pos.yy) {}
	constexpr position_t(room_location_t room, uint8_t xx, uint8_t yy) : coord_base(room.xx * 50 + xx, room.yy * 50 + yy) {}

	constexpr room_location_t room_location() const {
		return room_location_t(xx / 50, yy / 50);
	}

	constexpr local_position_t to_local() const {
		return local_position_t(xx % 50, yy % 50);
	}

	friend std::ostream& operator<<(std::ostream& os, position_t that);

	using neighbors_iterable = constexpr_neighbor_iteratable_t<position_t,
		direction_t::top_left, direction_t::top, direction_t::top_right,
		direction_t::right,
		direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
		direction_t::left
	>;
	constexpr neighbors_iterable neighbors() const {
		return neighbors_iterable(*this);
	}
};

inline constexpr position_t local_position_t::in_room(room_location_t room) const {
	return position_t(room, *this);
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
		local_matrix_store_t() = default;
		constexpr local_matrix_store_t(Type value) {
			fill(value);
		}

		constexpr void fill(Type value) {
			std::fill(costs.begin(), costs.end(), value);
		}

		constexpr reference operator[](uint16_t index) {
			return costs[index];
		}

		constexpr const_reference operator[](uint16_t index) const {
			return costs[index];
		}

		constexpr Store* data() {
			return costs.data();
		}
};

// Backing store for local_matrix_t which can bitpack 1, 2, or 4 bits into a matrix
template <typename Type, typename Store, size_t Pack>
class local_matrix_store_t<Type, Store, Pack, true> {
	protected:
		static constexpr uint8_t log2(size_t value) {
			return value == 1 ? 0 : (1 + log2(value >> 1));
		}
		static constexpr uint8_t StoreBits = sizeof(Store) << 3;
		static constexpr uint8_t IndexShift = log2(StoreBits / Pack);
		static constexpr uint8_t IndexMask = StoreBits / Pack - 1;
		static constexpr uint8_t IndexBitShift = log2(Pack);
		static constexpr Store Mask = (1 << Pack) - 1;

		static_assert(StoreBits >= Pack, "sizeof(Store) must be greater than or equal to pack bits");
		static_assert(StoreBits % Pack == 0, "Store is not aligned to pack bits");

		std::array<Store, (2500 * Pack + StoreBits / Pack) / StoreBits> costs;

		class reference {
			private:
				Store& ref;
				uint8_t bit_pos;

			public:
				constexpr reference(uint8_t bit_pos, Store& ref) : ref(ref), bit_pos(bit_pos) {}
				constexpr operator Type() const {
					return (ref >> bit_pos) & Mask;
				}
				constexpr const reference& operator=(Type value) const {
					ref = (ref & ~(Mask << bit_pos)) | ((value & Mask) << bit_pos);
					return *this;
				}
		};
		using const_reference = Type;

	public:
		local_matrix_store_t() = default;
		constexpr local_matrix_store_t(Type value) {
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

		constexpr reference operator[](uint16_t index) {
			return reference((index & IndexMask) << IndexBitShift, costs[index >> IndexShift]);
		}

		constexpr const_reference operator[](uint16_t index) const {
			return (costs[index >> IndexShift] >> ((index & IndexMask) << IndexBitShift)) & Mask;
		}

		constexpr Store* data() {
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

		constexpr reference get(uint8_t xx, uint8_t yy) {
			return (*this)[xx * 50 + yy];
		}

		constexpr const_reference get(uint8_t xx, uint8_t yy) const {
			return (*this)[xx * 50 + yy];
		}

		constexpr void set(uint8_t xx, uint8_t yy, Type cost) {
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
/*
template <typename Derived, typename Integral, typename IntegralUnion>
struct std::hash<screeps::coord_base_t<Derived, Integral, IntegralUnion>> {
	auto operator()(screeps::coord_base_t<Derived, Integral, IntegralUnion> val) const {
		return std::hash<IntegralUnion>()(val.id);
	}
};
*/
template <> struct std::hash<screeps::room_location_t> {
	auto operator()(screeps::room_location_t val) const {
		return std::hash<decltype(screeps::room_location_t().id)>()(val.id);
	}
};
template <> struct std::hash<screeps::local_position_t> {
	auto operator()(screeps::local_position_t val) const {
		return std::hash<decltype(screeps::local_position_t().id)>()(val.id);
	}
};
template <> struct std::hash<screeps::position_t> {
	auto operator()(screeps::position_t val) const {
		return std::hash<decltype(screeps::position_t().id)>()(val.id);
	}
};

