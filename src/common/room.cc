#include <screeps/room.h>

namespace screeps {

void room_t::update_pointers() {
	construction_sites = {nullptr, nullptr};
	flags = {nullptr, nullptr};
	if (mineral != nullptr) {
		mineral = &mineral_holder;
	}
	controller = nullptr;
/*
	storage = nullptr;
	terminal = nullptr;
*/
	for (auto& structure : structures) {
		if (auto controller = structure.get<controller_t>(); controller != nullptr) {
			this->controller = controller;
		}
/*
			case structure_t::storage:
				storage = &structure;
				break;
			case structure_t::terminal:
				terminal = &structure;
				break;
*/
	}
}

int room_t::create_construction_site(position_t pos, structure_t::type_t structure_type, const std::string& name) const {
	if (pos.room_location() != location) {
		// JS @screeps/engine doesn't actually check this..
		return k_err_invalid_args;
	}
	return create_construction_site(pos.to_local(), structure_type, name);
}

} // namespace screeps
