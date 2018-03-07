#pragma once
#include "./array.h"
#include <iostream>
#include <string>

template <int Capacity>
struct string_t : array_t<char, Capacity> {
	string_t() = default;

	string_t(const char* data) : array_t<char, Capacity>(strlen(data), data) {}

	operator std::string() const {
		return std::string(this->data(), this->size());
	}

	friend std::ostream& operator<< (std::ostream& os, const string_t<Capacity>& that) {
		os.write(that.data(), that.size());
		return os;
	}
};
