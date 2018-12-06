#include <screeps/cpu.h>
#include <emscripten.h>

static screeps::cpu::native_heap_t heap_status;

extern "C" {

extern void* emscripten_builtin_malloc(size_t size);
extern void emscripten_builtin_free(void* ptr);
extern size_t malloc_usable_size(void* ptr);

void* malloc(size_t size) {
	void* ptr = emscripten_builtin_malloc(size);
	heap_status.size += malloc_usable_size(ptr);
	++heap_status.allocs;
	return ptr;
}

void free(void* ptr) {
	heap_status.size -= malloc_usable_size(ptr);
	++heap_status.frees;
	emscripten_builtin_free(ptr);
}

}

namespace screeps::cpu {
	const native_heap_t& get_native_heap_statistics() {
		return heap_status;
	}

	EMSCRIPTEN_KEEPALIVE
	void adjust_memory_size(int memory) {
		heap_status.size += memory;
	}

} // namespace screeps::cpu
