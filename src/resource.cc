#include <screeps/resource.h>
#include <emscripten.h>

namespace screeps {

void resource_store_t::init() {
	EM_ASM({
		Module.screeps.resource.init($0, $1);
	}, &extended_stores, sizeof(extended_resource_store_t));
}

void resource_store_t::preloop() {
	extended_stores.clear();
}

} // namespace screeps
