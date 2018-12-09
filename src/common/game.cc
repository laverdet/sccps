#include <screeps/game.h>

namespace screeps {

void game_state_t::update_pointers() {
	construction_site_t* current = nullptr;
	room_location_t current_location;
	for (auto& site : construction_sites) {
		auto location = site.pos.room_location();
		if (current == nullptr) {
			current = &site;
			current_location = location;
		}
		if (&site == &construction_sites.back() || location != current_location) {
			auto& [location, room] = *rooms.find(site.pos.room_location());
			room.construction_sites = {current, &site + 1};
			current = nullptr;
		}
	}
}

std::ostream& operator<<(std::ostream& os, const game_object_t& that) {
	return os <<"game_object_t[" <<that.id <<"]";
}

} // namespace screeps
