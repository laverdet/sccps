#pragma once
#include <array>
#include <cstdint>
#include "./array.h"

constexpr int kExtraStoreReservation = 100;

// Resource types
enum struct resource_t {
	none,
	// The basics
	energy, power,
	// Base minerals
	H, O, U, L, K, Z, X, G,
	// Base compounds
	OH, ZK, UL,
	// Tier 1 compounds
	UH, UO, KH, KO, LH, LO, ZH, ZO, GH, GO,
	// Tier 2 compounds
	UH2O, UHO2, KH2O, KHO2, LH2O, LHO2, ZH2O, ZHO2, GH2O, GHO2,
	// Tier 3 compounds
	XUH2O, XUHO2, XKH2O, XKHO2, XLH2O, XLHO2, XZH2O, XZHO2, XGH2O, XGHO2,
	// Marker for number of resource types
	size,
};

// `store` or `carry` on game objects. Optimized in the case <=1 resource type is in the store, and
// will reference an external location if more resources are stored.
struct resource_store_t {
	public:
		using value_type = uint32_t;

	private:
		using extended_resource_store_t = std::array<value_type, (int)resource_t::size>;
		extended_resource_store_t* extended;
		resource_t single_type;
		value_type single_amount;

		static array_t<extended_resource_store_t, kExtraStoreReservation> extended_stores;
		static const value_type zero;

		void promote() {
			extended = &extended_stores.emplace_back();
			(*extended)[(int)single_type] = single_amount;
		}

		template <typename value_type, typename store_type>
		class iterator_base {
			private:
				struct reference {
					const resource_t type;
					value_type& amount;
					reference(resource_t type, value_type& amount) : type(type), amount(amount) {}
				};
				store_type& store;
				reference val;

				void increment() {
					if (store.extended == nullptr) {
						const_cast<resource_t&>(val.type) = resource_t::size;
					} else {
						resource_t type = val.type;
						do {
							type = static_cast<resource_t>((int)type + 1);
						} while (type != resource_t::size && (*store.extended)[(int)type] == 0);
						// horrifying
						val.~reference();
						new(&val) reference(type, (*store.extended)[(int)type]);
					}
				}

			public:
				using iterator_category = std::forward_iterator_tag;
				iterator_base(
					store_type& store,
					resource_t type
				) :
					store(store),
					val(type, store.extended == nullptr ? store.single_amount : (*store.extended)[0]) {
					if (store.extended != nullptr && type != resource_t::size && (*store.extended)[(int)type] == 0) {
						increment();
					}
				}
				iterator_base(const iterator_base&) = default;
				iterator_base& operator=(const iterator_base&) = default;
				~iterator_base() = default;

				iterator_base& operator++() {
					increment();
					return *this;
				}

				iterator_base operator++(int) {
					iterator_base copy(*this);
					increment();
					return copy;
				}

				reference& operator*() {
					return val;
				}

				bool operator==(const iterator_base& rhs) const {
					return val.type == rhs.val.type;
				}

				bool operator!=(const iterator_base& rhs) const {
					return val.type != rhs.val.type;
				}
		};

	public:
		using iterator = iterator_base<value_type, resource_store_t>;
		using const_iterator = iterator_base<const value_type, const resource_store_t>;

		static void init();
		static void preloop();

		resource_store_t() : extended(nullptr), single_type(resource_t::energy), single_amount(0) {}

		value_type& operator[](resource_t type) {
			if (extended == nullptr) {
				if (single_type == type) {
					return single_amount;
				} else if (single_amount == 0) {
					single_type = type;
					return single_amount;
				} else {
					promote();
				}
			}
			return (*extended)[(int)type];
		}

		const value_type& operator[](resource_t type) const {
			if (extended == nullptr) {
				if (single_type == type) {
					return single_amount;
				} else {
					return zero;
				}
			}
			return (*extended)[(int)type];
		}

		value_type sum() const {
			if (extended == nullptr) {
				return single_amount;
			} else {
				value_type sum = 0;
				for (auto& ii : *extended) {
					sum += ii;
				}
				return sum;
			}
		}

		iterator begin() {
			return iterator(*this, extended == nullptr ? single_type : resource_t::energy);
		}

		iterator end() {
			return iterator(*this, resource_t::size);
		}

		const_iterator begin() const {
			return const_iterator(*this, extended == nullptr ? single_type : resource_t::energy);
		}

		const_iterator end() const {
			return const_iterator(*this, resource_t::size);
		}
};
