#include <screeps/cpu.h>

static screeps::cpu::native_heap_t heap_status;

namespace screeps::cpu {
	const native_heap_t& get_native_heap_statistics() {
		return heap_status;
	}
} // namespace screeps::cpu
