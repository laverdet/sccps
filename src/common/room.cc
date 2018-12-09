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
		switch (structure.type) {
			case structure_t::controller:
				controller = &structure.controller;
				break;
/*
			case structure_t::storage:
				storage = &structure;
				break;
			case structure_t::terminal:
				terminal = &structure;
				break;
*/
			default:;
		}
	}
}

} // namespace screeps
