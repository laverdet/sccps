#include <screeps/structure.h>

namespace screeps {

int spawn_t::spawn_creep(const creep_body_t& /* body */, const std::string& name) const {
	std::cerr <<*this <<".spawn_creep(..., \"" <<name <<"\")\n";
	return 0;
}

} // namespace screeps
