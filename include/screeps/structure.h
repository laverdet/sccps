#pragma once
#include "./creep.h"
#include "./object.h"
#include "./internal/js_handle.h"
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <optional>
#include <vector>

namespace screeps {

/**
 * unowned structures:
 * container, road, wall, portal
 */

struct structure_t : game_object_t {
	enum type_t {
		none,
		container,
		controller,
		extension,
		extractor,
		keeper_lair,
		lab,
		link,
		nuker,
		observer,
		portal,
		power_bank,
		power_spawn,
		rampart,
		road,
		spawn,
		storage,
		terminal,
		tower,
		wall,
		size
	};

	type_t type; // must be first!

	static void init();
	friend std::ostream& operator<<(std::ostream& os, const structure_t& that);
};

struct destroyable_structure_t : structure_t {
	int32_t hits;
	int32_t hits_max;
};

struct owned_structure_t : destroyable_structure_t {
	bool my;
};

struct controller_t : structure_t {
	int32_t level;
	int32_t progress;
	int32_t progress_total;
	int32_t ticks_to_downgrade;
	int32_t upgrade_blocked;
};

struct extension_t : owned_structure_t {
	int32_t energy;
	int32_t energy_capacity;
};

struct _spawn_structs_t {
	// Stores `body` option for `spawn_creep`. Body description is not available to C++ after
	// construction.
	struct body_t : internal::js_handle_t {
		body_t() = default;
		explicit body_t(const std::vector<bodypart_t>& parts);

		template <class Iterator>
		body_t(Iterator begin, Iterator end) : body_t(std::vector<bodypart_t>(begin, end)) {}

		template <class Container>
		explicit body_t(const Container& container) : body_t(container.begin(), container.end()) {}

		body_t(std::initializer_list<bodypart_t> list) : body_t(list.begin(), list.end()) {}
	};

	// Stores spawning direction in a single 32-bit integer
	class directions_t {
		friend struct spawn_t;
		private:
			uint32_t bits = 0;
		public:
			class iterator : public forward_iterator_t<iterator> {
				private:
					uint32_t bits;
				public:
					explicit constexpr iterator(uint32_t bits) : bits(bits) {}
					constexpr direction_t operator*() const {
						return static_cast<direction_t>(bits & 0x0f);
					}
					constexpr bool operator==(const iterator& rhs) const {
						return bits == rhs.bits;
					}
					constexpr iterator& operator++() {
						bits >>= 4;
						return *this;
					}
			};

			constexpr directions_t() = default;

			template <class Iterator>
			constexpr directions_t(Iterator begin, Iterator end) {
				int length = std::distance(begin, end);
				if (length > 8) {
					throw std::range_error("spawn_t::directions_t");
				}
				while (begin != end) {
					bits >>= 4;
					bits |= +*begin << 28;
					++begin;
				}
				bits >>= (8 - length) * 4;
			}

			constexpr directions_t(std::initializer_list<direction_t> list) : directions_t(list.begin(), list.end()) {}

			constexpr iterator begin() const {
				return iterator(bits);
			}
			constexpr iterator end() const {
				return iterator(0);
			}
	};

	struct options_t {
		directions_t directions;
	};
};

struct spawn_t : owned_structure_t {
	using body_t = _spawn_structs_t::body_t;
	using directions_t = _spawn_structs_t::directions_t;
	using options_t = _spawn_structs_t::options_t;

	struct spawning_t {
		directions_t directions;
		int need_time;
		int remaining_time;
		creep_t::name_t name;

		void cancel() const;
		void set_directions(directions_t directions) const;

		spawn_t& spawn() {
			return const_cast<spawn_t&>(const_cast<const spawning_t*>(this)->spawn());
		}
		const spawn_t& spawn() const {
			return *reinterpret_cast<const spawn_t*>(reinterpret_cast<const char*>(this) - offsetof(spawn_t, spawning));
		}

		private:
			friend spawn_t;
			spawning_t() = default;
	};

	int32_t energy;
	int32_t energy_capacity;
	union {
		struct {
			spawning_t _spawning;
			bool _is_spawning;
		};
		std::optional<spawning_t> spawning;
	};

	spawn_t() {}
	int spawn_creep(const body_t& body, const std::string& name, const options_t& options = {}) const;
};

// Used to reserve enough space in the array memory for any structure
union structure_union_t {
	struct {
		position_t pos;
		sid_t id;
		structure_t::type_t type;
	};

	destroyable_structure_t destroyable;
	owned_structure_t owned;

	controller_t controller;
	extension_t extension;
	spawn_t spawn;

	structure_union_t() {}; // NOLINT(modernize-use-equals-default)

	template <class Memory>
	void serialize(Memory& memory) {
		memory.copy(reinterpret_cast<uint8_t*>(this), sizeof(structure_union_t));
	}

	bool is_owned() const {
		switch (type) {
			case structure_t::extension:
			case structure_t::extractor:
			case structure_t::lab:
			case structure_t::link:
			case structure_t::nuker:
			case structure_t::observer:
			case structure_t::power_spawn:
			case structure_t::rampart:
			case structure_t::spawn:
			case structure_t::storage:
			case structure_t::terminal:
			case structure_t::tower:
			case structure_t::wall:
				return true;
			default:
				return false;
		}
	}

	operator game_object_t&() { // NOLINT(hicpp-explicit-conversions)
		return *reinterpret_cast<game_object_t*>(this);
	}

	operator const game_object_t&() const { // NOLINT(hicpp-explicit-conversions)
		return *reinterpret_cast<const game_object_t*>(this);
	}
};

} // namespace screeps
