#include "./javascript.h"
#include <screeps/resource.h>

namespace screeps {

void resource_store_t::init() {
	EM_ASM({
		Module.screeps.object.initResourceStoreLayout($0, $1);
	}, sizeof(extended_resource_store_t), 0);
}

} // namespace screeps
