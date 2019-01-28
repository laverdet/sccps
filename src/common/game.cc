#include <screeps/game.h>
#include <iostream>

namespace screeps {

void game_state_t::clear_indices() {
	construction_sites_by_id.clear();
	creeps_by_id.clear();
	creeps_by_name.clear();
	dropped_resources_by_id.clear();
	flags_by_name.clear();
	sources_by_id.clear();
	structures_by_id.clear();
	tombstones_by_id.clear();
}

void game_state_t::update_pointers() {
	update_pointer_container<&room_t::construction_sites>(construction_sites);
	update_pointer_container<&room_t::flags>(flags);
}

template <auto Property, class Container>
void game_state_t::update_pointer_container(Container& container) {
	typename Container::value_type* current = nullptr;
	room_location_t current_location;
	for (auto& object : container) {
		auto location = object.pos.room_location();
		if (current == nullptr) {
			current = &object;
			current_location = location;
		}
		if (&object == &container.back() || location != current_location) {
			auto ii = rooms.find(object.pos.room_location());
			if (ii != rooms.end()) {
				auto& [location, room] = *ii;
				room.*Property = {current, &object + 1};
				current = nullptr;
			}
		}
	}
}

std::ostream& operator<<(std::ostream& os, const game_object_t& that) {
	return os <<"game_object_t[" <<that.id <<"]";
}

} // namespace screeps
