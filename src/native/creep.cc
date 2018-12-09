#include <screeps/creep.h>
#include <iostream>

namespace screeps {

int creep_t::build(const game_object_t& target) const {
	std::cerr <<*this <<".build(" <<target <<")\n";
	return 0;
}

int creep_t::cancel_order(const char* method) const {
	std::cerr <<*this <<".cancel_order(" <<method <<")\n";
	return 0;
}

int creep_t::drop(resource_t resource, int amount) const {
	std::cerr <<*this <<".drop(" <<resource <<", " <<amount <<")\n";
	return 0;
}

int creep_t::harvest(const game_object_t& target) const {
	std::cerr <<*this <<".harvest(" <<target <<")\n";
	return 0;
}

int creep_t::move(direction_t direction) const {
	std::cerr <<*this <<".move(" <<direction <<")\n";
	return 0;
}

int creep_t::pickup(const game_object_t& target) const {
	std::cerr <<*this <<".pickup(" <<target <<")\n";
	return 0;
}

int creep_t::repair(const game_object_t& target) const {
	std::cerr <<*this <<".repair(" <<target <<")\n";
	return 0;
}

int creep_t::suicide() const {
	std::cerr <<*this <<".suicide()\n";
	return 0;
}

int creep_t::transfer(const game_object_t& target, resource_t resource, int amount) const {
	std::cerr <<*this <<".transfer(" <<target <<", " <<resource <<", " <<amount <<")\n";
	return 0;
}

int creep_t::upgrade_controller(const game_object_t& target) const {
	std::cerr <<*this <<".upgrade_controller(" <<target <<")\n";
	return 0;
}

//int creep_t::withdraw(const game_object_t& target, resource_t resource, int amount = -1) const;

} // namespace screeps
