#include <screeps/internal/js_handle.h>
#include <emscripten.h>
namespace screeps::internal {

js_handle_t::js_handle_ref_t::~js_handle_ref_t() {
	EM_ASM({
		Module.screeps.util.handleDtor($0);
	}, this->ref);
}

} // namespace screeps
