#include <screeps/cpu.h>

static screeps::cpu::native_heap_t heap_status;

#ifdef __APPLE__
#include <mach/mach.h>
#include <malloc/malloc.h>
static void*(*default_malloc)(malloc_zone_t* zone, size_t size);
static void(*default_free)(malloc_zone_t* zone, void* ptr);
static void(*default_free_definite_size)(malloc_zone_t* zone, void* ptr, size_t size);

struct zone_malloc_overwrite_t {
	static void* zone_malloc(malloc_zone_t* zone, size_t size) {
		void* ptr = default_malloc(zone, size);
		heap_status.size += zone->size(zone, ptr);
		++heap_status.allocs;
		return ptr;
	}

	static void zone_free(malloc_zone_t* zone, void* ptr) {
		heap_status.size -= zone->size(zone, ptr);
		++heap_status.frees;
		default_free(zone, ptr);
	}

	static void zone_free_definite_size(malloc_zone_t* zone, void* ptr, size_t size) {
		heap_status.size -= size;
		++heap_status.frees;
		default_free_definite_size(zone, ptr, size);
	}

	zone_malloc_overwrite_t() {
		malloc_zone_t* default_zone = malloc_default_zone();
		default_malloc = default_zone->malloc;
		default_free = default_zone->free;
		default_free_definite_size = default_zone->free_definite_size;
		mach_port_t self = mach_task_self();
		vm_protect(self, (vm_address_t)default_zone, sizeof(malloc_zone_t), 0, VM_PROT_READ | VM_PROT_WRITE);
		default_zone->malloc = zone_malloc;
		default_zone->free = zone_free;
		default_zone->free_definite_size = zone_free_definite_size;
		vm_protect(self, (vm_address_t)default_zone, sizeof(malloc_zone_t), 0, VM_PROT_READ);
	}
};
static zone_malloc_overwrite_t ctor;
#endif

namespace screeps::cpu {
	const native_heap_t& get_native_heap_statistics() {
		return heap_status;
	}
} // namespace screeps::cpu
