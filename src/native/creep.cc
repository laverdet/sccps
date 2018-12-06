#include <screeps/creep.h>

namespace screeps {

int creep_t::build(const game_object_t& target) const {
	return 0;
}

int creep_t::cancel_order(const char* method) const {
	return 0;
}

int creep_t::drop(resource_t resource, int amount) const {
	return 0;
}

int creep_t::harvest(const game_object_t& target) const {
	return 0;
}

int creep_t::move(direction_t direction) const {
	return 0;
}

int creep_t::pickup(const game_object_t& target) const {
	return 0;
}

int creep_t::repair(const game_object_t& target) const {
	return 0;
}

int creep_t::suicide() const {
	return 0;
}

int creep_t::transfer(const game_object_t& target, resource_t resource, int amount) const {
	return 0;
}

int creep_t::upgrade_controller(const game_object_t& target) const {
	return 0;
}

//int creep_t::withdraw(const game_object_t& target, resource_t resource, int amount = -1) const;

} // namespace screeps
