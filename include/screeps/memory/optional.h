#pragma once
#include <optional>

namespace serialization {

template <class Memory, class Type>
void read(Memory& memory, std::optional<Type>& value) {
	bool has_value;
	memory & has_value;
	if (has_value) {
		value = memory.read();
	} else {
		value = std::nullopt;
	}
}

template <class Memory, class Type>
void write(Memory& memory, const std::optional<Type>& value) {
	if (value) {
		memory & true;
		memory & *value;
	} else {
		memory & false;
	}
}

} // namespace serialization
