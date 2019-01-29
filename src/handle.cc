#include "./javascript.h"
#include <screeps/internal/js_handle.h>
#ifdef BUILDING_NODE_EXTENSION
#include <v8.h>
#endif

namespace screeps::internal {

#ifdef BUILDING_NODE_EXTENSION
js_handle_t::js_handle_ref_t::js_handle_ref_t(int ref) : ref(ref), isolate(v8::Isolate::GetCurrent()) {}
#else
js_handle_t::js_handle_ref_t::js_handle_ref_t(int ref) : ref(ref) {}
#endif

js_handle_t::js_handle_ref_t::~js_handle_ref_t() {
#ifdef BUILDING_NODE_EXTENSION
	if (isolate != v8::Isolate::GetCurrent()) {
		return;
	}
#endif
#ifdef JAVASCRIPT
	EM_ASM({
		Module.screeps.util.handleDtor($0);
	}, this->ref);
#endif
}

} // namespace screeps::internal
