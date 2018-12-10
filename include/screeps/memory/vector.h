#pragma once
#include <vector>

namespace serialization {

template <class Memory, class Type, class Allocator>
void read(Memory& memory, std::vector<Type, Allocator>& vector) {
	size_t size;
	memory & size;
	vector.reserve(size);
	while (size-- > 0) {
		vector.emplace_back(memory.read());
	}
}

template <class Memory, class Type, class Allocator>
void write(Memory& memory, const std::vector<Type, Allocator>& vector) {
	memory & vector.size();
	for (auto& element : vector) {
		memory & element;
	}
}

} // namespace serialization
