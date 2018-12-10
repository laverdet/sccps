#pragma once
#include "./pair.h"
#include <unordered_map>

namespace serialization {

template <class Memory, class Key, class Value, class Hash, class KeyEqual, class Allocator>
void read(Memory& memory, std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>& map) {
	size_t size;
	memory & size;
	map.reserve(size);
	while (size-- > 0) {
		map.emplace(memory.read());
	}
}

template <class Memory, class Key, class Value, class Hash, class KeyEqual, class Allocator>
void write(Memory& memory, const std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>& map) {
	memory & map.size();
	for (auto& pair : map) {
		memory & pair;
	}
}

} // namespace serialization
