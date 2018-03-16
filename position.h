#pragma once
#include "./iterator.h"
#include <array>
#include <iostream>
#include <limits>
#include <unordered_map>
constexpr int kWorldSize = 255;

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

// This is replacement for `roomName` in the JS API.
// "E0S0" -> { xx: 128, yy: 128 }
struct room_location_t {
	union {
		uint16_t id;
		struct {
			uint8_t xx, yy;
		};
	};
	static constexpr size_t max = std::numeric_limits<decltype(id)>::max();

	room_location_t() = default;
	constexpr room_location_t(uint16_t xx, uint16_t yy) : xx(xx), yy(yy) {}
	constexpr room_location_t(const room_location_t& that) : xx(that.xx), yy(that.yy) {}

	const class terrain_t& terrain() const;

	constexpr bool operator==(const room_location_t& rhs) const {
		return id == rhs.id;
	}

	constexpr bool operator!=(const room_location_t& rhs) const {
		return id != rhs.id;
	}

	friend std::ostream& operator<< (std::ostream& os, const room_location_t& that);

	struct circle_t {
		double radius = 0.15;
		uint32_t fill = 0xffffff;
		double opacity = 0.5;
		uint32_t stroke = 0xff000000;
		double stroke_width = 0.1;
		// line_style
	};
	void draw_circle(double xx, double yy, const circle_t& options) const;
};

// Hash specialization for room_location_t
template <> struct std::hash<room_location_t> {
	size_t operator()(const room_location_t& val) const {
		return std::hash<uint16_t>()(val.id);
	}
};

// Simple container for a location in an arbitrary room
struct local_position_t {
	union {
		uint16_t _id;
		struct {
			int8_t xx;
			int8_t yy;
		};
	};

	local_position_t() = default;

	constexpr local_position_t(int8_t xx, int8_t yy) : xx(xx), yy(yy) {
		if (xx < 0 || xx > 49 || yy < 0 || yy > 49) {
			throw std::logic_error("Invalid local_position_t");
		}
	}

	friend std::ostream& operator<< (std::ostream& os, const local_position_t& that);

	constexpr bool operator==(const local_position_t& rhs) const {
		return _id == rhs._id;
	}

	constexpr bool operator!=(const local_position_t& rhs) const {
		return _id != rhs._id;
	}

	uint8_t distance_to(local_position_t that) const {
		return std::max(std::abs(xx - that.xx), std::abs(yy - that.yy));
	}

	constexpr local_position_t in_direction(direction_t direction) const {
		switch (direction) {
			case direction_t::top:
				return local_position_t(xx, yy - 1);
			case direction_t::top_right:
				return local_position_t(xx + 1, yy - 1);
			case direction_t::right:
				return local_position_t(xx + 1, yy);
			case direction_t::bottom_right:
				return local_position_t(xx + 1, yy + 1);
			case direction_t::bottom:
				return local_position_t(xx, yy + 1);
			case direction_t::bottom_left:
				return local_position_t(xx - 1, yy + 1);
			case direction_t::left:
				return local_position_t(xx - 1, yy);
			case direction_t::top_left:
				return local_position_t(xx - 1, yy - 1);
			default:
				throw std::logic_error("Invalid direction");
		}
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
					if (xx < 0 || yy < 0) {
						++(*this);
					}
				}

				constexpr local_position_t operator*() const {
					return local_position_t(xx, yy);
				}

				constexpr bool operator==(const iterator rhs) const {
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
					if ((xx < 0 || xx > 49 || yy < 0 || yy > 49) && tick < range2 * 8) {
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

				constexpr bool operator==(const iterator rhs) const {
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

				constexpr bool operator==(const iterator rhs) const {
					return ii != rhs.ii;
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

// Hash specialization for local_position_t
template <> struct std::hash<local_position_t> {
	size_t operator()(const local_position_t& val) const {
		return std::hash<uint16_t>()(val._id);
	}
};

// This represents a position in a continuous plane of the whole world. `roomName` is implied from
// xx and yy.
struct position_t {
	using iterable = constexpr_neighbor_iteratable_t<position_t,
		direction_t::top_left, direction_t::top, direction_t::top_right,
		direction_t::right,
		direction_t::bottom_right, direction_t::bottom, direction_t::bottom_left,
		direction_t::left
	>;
	union {
		uint32_t _id;
		struct {
			uint16_t xx, yy;
		};
	};
	position_t() = default;
	constexpr position_t(uint16_t xx, uint16_t yy) : xx(xx), yy(yy) {}
	constexpr position_t(room_location_t room, local_position_t pos) : xx(room.xx * 50 + pos.xx), yy(room.yy * 50 + pos.yy) {}
	constexpr position_t(room_location_t room, uint8_t xx, uint8_t yy) : xx(room.xx * 50 + xx), yy(room.yy * 50 + yy) {}

	constexpr room_location_t room_location() const {
		return room_location_t(xx / 50, yy / 50);
	}

	constexpr local_position_t to_local() const {
		return local_position_t(xx % 50, yy % 50);
	}

	friend std::ostream& operator<< (std::ostream& os, const position_t& that);

	constexpr bool operator==(const position_t& rhs) const {
		return _id == rhs._id;
	}

	constexpr bool operator!=(const position_t& rhs) const {
		return _id != rhs._id;
	}

	uint32_t distance_to(position_t that) const {
		return std::max(std::abs(xx - that.xx), std::abs(yy - that.yy));
	}

	constexpr position_t in_direction(direction_t direction) const {
		switch (direction) {
			case direction_t::top:
				return position_t(xx, yy - 1);
			case direction_t::top_right:
				return position_t(xx + 1, yy - 1);
			case direction_t::right:
				return position_t(xx + 1, yy);
			case direction_t::bottom_right:
				return position_t(xx + 1, yy + 1);
			case direction_t::bottom:
				return position_t(xx, yy + 1);
			case direction_t::bottom_left:
				return position_t(xx - 1, yy + 1);
			case direction_t::left:
				return position_t(xx - 1, yy);
			case direction_t::top_left:
				return position_t(xx - 1, yy - 1);
		}
		throw std::logic_error("Invalid direction");
	}

	constexpr iterable neighbors() const {
		return iterable(*this);
	}
};

// Hash specialization for position_t
template <> struct std::hash<position_t> {
	size_t operator()(const position_t& val) const {
		return std::hash<uint32_t>()(val._id);
	}
};

// Matrix of any type which holds a value for each position in a room
template <typename T>
class local_matrix_store_t {
	private:
		std::array<T, 2500> costs;

	public:
		local_matrix_store_t() {}

		local_matrix_store_t(T value) {
			std::fill(costs.begin(), costs.end(), value);
		}

		constexpr T get(uint16_t index) const {
			return costs[index];
		}

		constexpr void set(uint16_t index, T cost) {
			costs[index] = cost;
		}

		const T* data() const {
			return costs.data();
		}
};

template <typename T>
class local_matrix_t : public local_matrix_store_t<T> {
	public:
		using local_matrix_store_t<T>::local_matrix_store_t;
		using local_matrix_store_t<T>::get;
		using local_matrix_store_t<T>::set;

		constexpr T get(uint8_t xx, uint8_t yy) const {
			return local_matrix_store_t<T>::get(xx * 50 + yy);
		}

		constexpr T get(local_position_t pos) const {
			return get(pos.xx, pos.yy);
		}

		constexpr void set(uint8_t xx, uint8_t yy, T cost) {
			set(xx * 50 + yy, cost);
		}

		constexpr void set(local_position_t pos, T cost) {
			set(pos.xx, pos.yy, cost);
		}
};
