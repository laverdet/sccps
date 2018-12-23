#include <screeps/creep.h>
#include <iostream>

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

std::ostream& operator<<(std::ostream& os, bodypart_t type) {
	os <<"bodypart_t::";
	switch (type) {
		case bodypart_t::none: return os <<"none";
		case bodypart_t::attack: return os <<"attack";
		case bodypart_t::carry: return os <<"carry";
		case bodypart_t::claim: return os <<"claim";
		case bodypart_t::heal: return os <<"heal";
		case bodypart_t::move: return os <<"move";
		case bodypart_t::ranged_attack: return os <<"ranged_attack";
		case bodypart_t::tough: return os <<"tough";
		case bodypart_t::work: return os <<"work";
	}
}

} // namespace screeps
