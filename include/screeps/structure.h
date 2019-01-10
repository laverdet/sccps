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

	// nb: must match structure_union_t
	type_t type;
	int32_t hits;
	int32_t hits_max;
	int32_t owner;
	bool my; // TODO: this is has to go

	static void init();
	friend std::ostream& operator<<(std::ostream& os, const structure_t& that);
};

struct controller_t : structure_t {
	static constexpr auto k_type = structure_t::controller;
	int32_t level;
	int32_t progress;
	int32_t progress_total;
	int32_t ticks_to_downgrade;
	int32_t upgrade_blocked;
};

struct container_t : structure_t {
	static constexpr auto k_type = structure_t::container;
	resource_store_t store;
	int32_t store_capacity;
	int32_t ticks_to_decay;
};

struct extension_t : structure_t {
	static constexpr auto k_type = structure_t::extension;
	int32_t energy;
	int32_t energy_capacity;
};

struct road_t : structure_t {
	static constexpr auto k_type = structure_t::road;
	int32_t ticks_to_decay;
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

struct spawn_t : structure_t {
	static constexpr auto k_type = structure_t::spawn;
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

// Basically a std::variant<> but all types shared a common base class
union structure_union_t {
	public:
		struct {
			// nb: must match structure_t
			position_t pos;
			sid_t id;
			structure_t::type_t type;
			int32_t hits;
			int32_t hits_max;
			int32_t owner;
		};

	private:
		template <class... Types>
		struct union_t {};

		template <class Type, class... Types>
		struct union_t<Type, Types...> {
			union {
				Type value;
				union_t<Types...> rest;
			};

			template <class Structure, class Function, class... Args>
			decltype(auto) visit(Structure* structure, Function function, Args&&... args) const {
				if (structure->type == Type::k_type) {
					using cv_type = typename std::conditional<std::is_const_v<Structure>, const Type, Type>::type;
					return function(*reinterpret_cast<cv_type*>(structure), std::forward<Args>(args)...);
				} else if constexpr (sizeof...(Types) == 0) {
					throw std::invalid_argument("Invalid structure type");
				} else {
					return rest.visit(structure, function, std::forward<Args>(args)...);
				}
			}
		};

		union_t<controller_t, container_t, extension_t, road_t, spawn_t> store;

	public:
		structure_union_t() {}; // NOLINT(modernize-use-equals-default)

		template <class Memory>
		void serialize(Memory& memory) {
			memory.copy(reinterpret_cast<uint8_t*>(this), sizeof(structure_union_t));
		}

		operator structure_t&() { // NOLINT(hicpp-explicit-conversions)
			return *reinterpret_cast<structure_t*>(this);
		}

		operator const structure_t&() const { // NOLINT(hicpp-explicit-conversions)
			return *reinterpret_cast<const structure_t*>(this);
		}

		template <class Type>
		Type* get() {
			return const_cast<Type*>(const_cast<const structure_union_t*>(this)->get<Type>());
		}

		template <class Type>
		const Type* get() const {
			if (type == Type::k_type) {
				return reinterpret_cast<const Type*>(this);
			} else {
				return nullptr;
			}
		}

		template <class... Args>
		decltype(auto) visit(Args&&... args) {
			return store.visit(this, std::forward<Args>(args)...);
		}

		template <class... Args>
		decltype(auto) visit(Args&&... args) const {
			return store.visit(this, std::forward<Args>(args)...);
		}
};

} // namespace screeps
