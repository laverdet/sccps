#include <screeps/cpu.h>
#include "./javascript.h"

namespace {
	static screeps::cpu::native_heap_t heap_status;
}

namespace screeps::cpu {

const native_heap_t& get_native_heap_statistics() {
	return heap_status;
}

void halt() {
#ifdef JAVASCRIPT
	EM_ASM({ Game.cpu.halt(); });
#endif
	std::terminate();
}

} // namespace screeps::cpu


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
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
	EMSCRIPTEN_KEEPALIVE
	void adjust_memory_size(int memory) {
		heap_status.size += memory;
	}
} // namespace screeps::cpu

#elif __APPLE__
#include <mach/mach.h>
#include <malloc/malloc.h>
#include <mutex>
static void*(*default_malloc)(malloc_zone_t* zone, size_t size);
static void*(*default_realloc)(malloc_zone_t* zone, void* ptr, size_t size);
static void(*default_free)(malloc_zone_t* zone, void* ptr);
static void(*default_free_definite_size)(malloc_zone_t* zone, void* ptr, size_t size);
static std::recursive_mutex zone_mutex;
static int zone_nest_depth = 0;

struct zone_nest_guard_t {
	std::lock_guard<std::recursive_mutex> lock;
	bool root;
	zone_nest_guard_t() : lock(zone_mutex) {
		root = ++zone_nest_depth == 1;
	}
	~zone_nest_guard_t() {
		--zone_nest_depth;
	}
};

struct zone_malloc_overwrite_t {
	static void* zone_malloc(malloc_zone_t* zone, size_t size) {
		zone_nest_guard_t nest;
		void* ptr = default_malloc(zone, size);
		if (nest.root && ptr != nullptr) {
			heap_status.size += zone->size(zone, ptr);
			++heap_status.allocs;
		}
		return ptr;
	}

	static void* zone_realloc(malloc_zone_t* zone, void* ptr, size_t size) {
		zone_nest_guard_t nest;
		if (nest.root && ptr != nullptr) {
			++heap_status.frees;
			heap_status.size -= zone->size(zone, ptr);
		}
		void* new_ptr = default_realloc(zone, ptr, size);
		if (nest.root) {
			if (new_ptr == nullptr) {
				// `ptr` is still alive. very rare.
				--heap_status.frees;
				heap_status.size += zone->size(zone, ptr);
			} else {
				++heap_status.allocs;
				heap_status.size += zone->size(zone, new_ptr);
			}
		}
		return new_ptr;
	}

	static void zone_free(malloc_zone_t* zone, void* ptr) {
		zone_nest_guard_t nest;
		if (nest.root) {
			heap_status.size -= zone->size(zone, ptr);
			++heap_status.frees;
		}
		default_free(zone, ptr);
	}

	static void zone_free_definite_size(malloc_zone_t* zone, void* ptr, size_t size) {
		zone_nest_guard_t nest;
		if (nest.root) {
			assert(zone->size(zone, ptr) == size);
			heap_status.size -= size;
			++heap_status.frees;
		}
		default_free_definite_size(zone, ptr, size);
	}

	zone_malloc_overwrite_t() {
		malloc_zone_t* default_zone = malloc_default_zone();
		default_malloc = default_zone->malloc;
		default_realloc = default_zone->realloc;
		default_free = default_zone->free;
		default_free_definite_size = default_zone->free_definite_size;
		mach_port_t self = mach_task_self();
		vm_protect(self, (vm_address_t)default_zone, sizeof(malloc_zone_t), 0, VM_PROT_READ | VM_PROT_WRITE);
		default_zone->malloc = zone_malloc;
		default_zone->realloc = zone_realloc;
		default_zone->free = zone_free;
		default_zone->free_definite_size = zone_free_definite_size;
		vm_protect(self, (vm_address_t)default_zone, sizeof(malloc_zone_t), 0, VM_PROT_READ);
	}

	~zone_malloc_overwrite_t() {
		malloc_zone_t* default_zone = malloc_default_zone();
		mach_port_t self = mach_task_self();
		vm_protect(self, (vm_address_t)default_zone, sizeof(malloc_zone_t), 0, VM_PROT_READ | VM_PROT_WRITE);
		default_zone->malloc = default_malloc;
		default_zone->realloc = default_realloc;
		default_zone->free = default_free;
		default_zone->free_definite_size = default_free_definite_size;
		vm_protect(self, (vm_address_t)default_zone, sizeof(malloc_zone_t), 0, VM_PROT_READ);
	}
};
namespace { static zone_malloc_overwrite_t ctor; }

#endif
