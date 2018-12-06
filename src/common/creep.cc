#include <screeps/creep.h>

namespace screeps {

const std::vector<creep_active_bodypart_t> creep_t::get_active_bodyparts() const {
	std::vector<creep_active_bodypart_t> parts;
	parts.reserve(hits / 100);
	int active_hits = hits;
	for (auto& ii : body) {
		if (active_hits > 100) {
			parts.emplace_back(ii, 100);
			active_hits -= 100;
		} else {
			parts.emplace_back(ii, active_hits);
			break;
		}
	}
	return parts;
}

std::ostream& operator<<(std::ostream& os, const creep_t& that) {
	if (that.my) {
		return os <<"creep_t[" <<that.name <<"]";
	} else {
		return os <<"creep_t[!" <<that.id <<"]";
	}
}

} // namespace screeps
