#include <screeps/resource.h>
#include <emscripten.h>

namespace screeps {

void resource_store_t::init() {
	EM_ASM({
		Module.screeps.object.initResourceStoreLayout($0, $1);
	}, sizeof(extended_resource_store_t), &extended_stores);
}

} // namespace screeps
