#include <screeps/internal/js_handle.h>
namespace screeps::internal {

js_handle_t::js_handle_ref_t::js_handle_ref_t(int ref) : ref(ref) {}

js_handle_t::js_handle_ref_t::~js_handle_ref_t() {
}

} // namespace screeps
