#include <screeps/structure.h>

namespace screeps {

spawn_t::body_t::body_t(const std::vector<bodypart_t>& /* parts */) : internal::js_handle_t(0) {
}

int spawn_t::spawn_creep(const body_t& /* body */, const std::string& name, const options_t& /* options */) const {
	std::cerr <<*this <<".spawn_creep(..., \"" <<name <<"\")\n";
	return 0;
}

} // namespace screeps
