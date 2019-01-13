#include <screeps/structure.h>

namespace screeps {

spawn_t::body_t::body_t(const std::vector<bodypart_t>& /* parts */) : internal::js_handle_t(0) {
}

int spawn_t::spawn_creep(const body_t& /* body */, const std::string& name, const options_t& /* options */) const {
	std::cerr <<*this <<".spawn_creep(..., \"" <<name <<"\")\n";
	return 0;
}

void spawn_t::spawning_t::cancel() const {
	std::cerr <<this->spawn() <<".spawning.cancel()\n";
}

void spawn_t::spawning_t::set_directions(directions_t directions) const {
	std::cerr <<this->spawn() <<".spawning.set_directions(...)\n";
}

} // namespace screeps
