#pragma once
#include <array>
#include <cstdint>
#include "./array.h"

namespace screeps {

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
		struct reference {
			const resource_t type;
			value_type& amount;
			reference(resource_t type, value_type& amount) : type(type), amount(amount) {}
		};

	private:
		using extended_resource_store_t = std::array<value_type, (int)resource_t::size>;
		static inline array_t<extended_resource_store_t, kExtraStoreReservation> extended_stores;
		extended_resource_store_t* extended;
		resource_t single_type;
		value_type single_amount;

		void promote() {
			extended = &extended_stores.emplace_back();
			(*extended)[(int)single_type] = single_amount;
		}

		template <typename store_type, typename reference_type>
		class iterator_base {
			private:
				store_type& store;
				resource_t type;

			public:
				iterator_base(store_type& store, resource_t type) : store(store), type(type) {
					if (store.extended != nullptr && type != resource_t::size && (*store.extended)[(int)type] == 0) {
						++(*this);
					}
				}
				iterator_base(const iterator_base&) = default;
				iterator_base& operator=(const iterator_base&) = default;
				~iterator_base() = default;

				reference operator*() const {
					if (store.extended) {
						return reference(type, (*store.extended)[(int)type]);
					} else {
						return reference(type, store.single_amount);
					}
				}

				iterator_base& operator++() {
					if (store.extended == nullptr) {
						type = resource_t::size;
					} else {
						do {
							type = static_cast<resource_t>((int)type + 1);
						} while (type != resource_t::size && (*store.extended)[(int)type] == 0);
					}
					return *this;
				}

				iterator_base operator++(int) {
					iterator_base copy(*this);
					++(*this);
					return copy;
				}

				bool operator!=(const iterator_base& rhs) const {
					return type != rhs.val.type;
				}
		};

	public:
		using iterator = iterator_base<resource_store_t, reference>;
		using const_iterator = iterator_base<const resource_store_t, const reference>;

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

		value_type operator[](resource_t type) const {
			if (extended == nullptr) {
				if (single_type == type) {
					return single_amount;
				} else {
					return 0;
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

} // namespace screeps
