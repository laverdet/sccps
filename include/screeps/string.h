#pragma once
#include "./array.h"
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>

namespace screeps {

template <int Capacity>
struct string_t : array_t<char, Capacity> {
	string_t() = default;
	explicit string_t(const std::string& string) : array_t<char, Capacity>(string.size(), string.c_str()) {}
	explicit string_t(const char* data) : array_t<char, Capacity>(strlen(data), data) {}

	/*
	template <class Memory>
	void serialize(Memory& memory) {
		memory & this->size_;
		memory.copy(this->data(), this->size_);
	}
	*/

	operator std::string() const { // NOLINT(hicpp-explicit-conversions)
		return std::string(this->data(), this->size());
	}

	bool operator==(const string_t& that) const {
		if (this->size() != that.size()) {
			return false;
		}
		return std::memcmp(this->data(), that.data(), this->size()) == 0;
	}

	friend std::ostream& operator<<(std::ostream& os, const string_t<Capacity>& that) {
		os.write(that.data(), that.size());
		return os;
	}
};

} // namespace screeps

// Hash specialization
template <size_t Capacity> struct std::hash<screeps::string_t<Capacity>> {
	auto operator()(const screeps::string_t<Capacity>& val) const {
		return std::hash<std::string_view>()(std::string_view(val.data(), val.size()));
	}
};
