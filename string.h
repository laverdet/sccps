#pragma once
#include "./array.h"
#include "./memory.h"
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>

namespace screeps {

template <int Capacity>
struct string_t : array_t<char, Capacity> {
	string_t() = default;
	string_t(const std::string& string) : array_t<char, Capacity>(string.size(), string.c_str()) {}
	string_t(const char* data) : array_t<char, Capacity>(strlen(data), data) {}
	string_t(screeps::memory_t& memory) {
		size_t size;
		memory >>size;
		if (size > Capacity) {
			throw std::range_error("Memory string too large");
		}
		this->resize(size);
		memory.read(reinterpret_cast<uint8_t*>(this->data()), size);
	}

	operator std::string() const {
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

	friend screeps::memory_t& operator<<(screeps::memory_t& memory, const string_t<Capacity>& that) {
		memory <<that.size();
		memory.write(reinterpret_cast<uint8_t*>(that.data()), that.size());
		return memory;
	}
};

} // namespace screeps

// Hash specialization
template <size_t Capacity> struct std::hash<screeps::string_t<Capacity>> {
	auto operator()(const screeps::string_t<Capacity>& val) const {
		return std::hash<std::string_view>()(std::string_view(val.data(), val.size()));
	}
};
