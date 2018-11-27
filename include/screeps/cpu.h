#pragma once
#include <stddef.h>

namespace screeps::cpu {
	struct native_heap_t {
		int allocs = 0;
		int frees = 0;
		int size = 0;
	};

	const native_heap_t& get_native_heap_statistics();
}; // namespace screeps::cpu
