#include "./resource.h"
#include <emscripten.h>

void resource_store_t::init() {
	EM_ASM({
		Module.screeps.resource.init($0, $1);
	}, &extended_stores, sizeof(extended_resource_store_t));
}

void resource_store_t::preloop() {
	extended_stores.clear();
}

array_t<resource_store_t::extended_resource_store_t, kExtraStoreReservation> resource_store_t::extended_stores;
