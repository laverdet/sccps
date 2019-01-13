#pragma once
#include <array>
#include <cstdint>
#include <iosfwd>
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

std::ostream& operator<<(std::ostream& os, resource_t type);
inline constexpr int operator+(resource_t resource) noexcept {
	return static_cast<int>(resource);
}

struct energy_store_t {
	int32_t energy;
	int32_t energy_capacity;
};

// `store` or `carry` on game objects. Optimized in the case <=1 resource type is in the store, and
// will reference an external location if more resources are stored.
struct resource_store_t {
	private:
		using mapped_type = int32_t;
		struct secret_value_type {
			resource_t type;
			mapped_type amount;
		};

	public:
		int32_t capacity;
	private:
		secret_value_type single;

	public:
		struct value_type {
			const resource_t type;
			int32_t amount;
		};
		using iterator = value_type*;
		using const_iterator = const value_type*;

		template <class Memory>
		void serialize(Memory& memory) {
			memory & capacity & single.type & single.amount;
		}

		mapped_type& operator[](resource_t type) {
			if (single.amount == 0) {
				single.type = type;
				return single.amount;
			} else if (single.type == type) {
				return single.amount;
			} else {
				throw std::runtime_error("resource_t::operator[]");
			}
		}

		mapped_type operator[](resource_t type) const {
			// nb: In the case where this store has nothing then only `single_amount` will be set,
			// `single_type` will be garbage
			if (single.amount == 0 || single.type != type) {
				return 0;
			} else {
				return single.amount;
			}
		}

		size_t size() const {
			return end() - begin();
		}

		mapped_type sum() const {
			return single.amount;
		}

		iterator begin() {
			return reinterpret_cast<iterator>(&single);
		}

		iterator end() {
			if (single.amount == 0) {
				return begin();
			} else {
				return begin() + 1;
			}
		}

		const_iterator begin() const {
			return reinterpret_cast<const_iterator>(&single);
		}

		const_iterator end() const {
			if (single.amount == 0) {
				return begin();
			} else {
				return begin() + 1;
			}
		}
};

} // namespace screeps
